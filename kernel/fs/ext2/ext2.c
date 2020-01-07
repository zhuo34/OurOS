#include "ext2.h"

#include <driver/sd.h>
#include <driver/vga.h>

#include <ouros/error.h>
#include <ouros/fs/fscache.h>

static file_system_type fs_type_ext2 = {
	.name 				= 		"ext2",
	.get_sb 			= 		get_sb_ext2,
};

static super_block_op sb_op_ext2 = {
	.alloc_inode 		=		alloc_inode_ext2,
	.alloc_dentry 		=		alloc_dentry_ext2,
	.create_dentry 		= 		create_dentry_ext2,
};

static inode_op inode_op_ext2 = {
	.lookup 			= 		lookup_ext2,
	.expand 			= 		expand_ext2,
	.clear 				= 		clear_ext2,
	.delete 			= 		delete_ext2,
};

static dentry_op dentry_op_ext2 = {
	.compare 			= 		compare_ext2,
	.open 				= 		dentry_open_file,
};

static file_op file_op_ext2 = {
	.list_dir 			= 		list_dir_ext2,
	.close 				= 		close_ext2,
};

file_system_type* get_fs_type_ext2()
{
	return &fs_type_ext2;
}

super_block* get_sb_ext2(DWord base_sec)
{
	super_block* sb_ret = nullptr;

	// 申请缓存区读取扇区数据
	Byte *buf = (Byte*)kmalloc(SECTOR_SIZE * 1);
	if (buf == nullptr) {
		sb_ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	
	// 初始化ext2系统有关信息
	fs_info_ext2* fs_info = (fs_info_ext2*)kmalloc(sizeof(fs_info_ext2));
	if (fs_info == nullptr) {
		sb_ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	fs_info->info_1st_sb_sec = base_sec + 2;
	if (!sd_read_block(buf, fs_info->info_1st_sb_sec, 2)) {
		sb_ret = ERR_PTR(-ERROR_READ_SDCARD);
		goto exit;
	}
	super_block_ext2* sb_ext2 = (super_block_ext2*)buf;
	fs_info->info_inode_pgroup 	= sb_ext2->sb_inode_pgroup;
	fs_info->info_inode_size 	= sb_ext2->sb_inode_size;
	if (sb_ext2->sb_blk_shift == 0 || sb_ext2->sb_blk_shift == 1) {
		fs_info->info_1st_gdt_sec = base_sec + 4;
	} else {
		fs_info->info_1st_gdt_sec = base_sec + (( BLOCK_SIZE_EXT2 << sb_ext2->sb_blk_shift) >> SECTOR_SHIFT);
	}

	// 构建超级块super_block
	sb_ret = (super_block*)kmalloc(sizeof(super_block));
	if (sb_ret == nullptr) {
		sb_ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	sb_ret->s_op 			= &sb_op_ext2;
	sb_ret->s_type 			= &fs_type_ext2;
	sb_ret->s_blocksize 	= BLOCK_SIZE_EXT2 << sb_ext2->sb_blk_shift;
	sb_ret->s_blockbits 	= SECTOR_SHIFT + 1 + sb_ext2->sb_blk_shift;
	sb_ret->s_info_ext2 	= fs_info;
	sb_ret->s_basesec 		= base_sec;
	INIT_LIST_HEAD(&sb_ret->s_listnode);

	// 初始化根目录inode
	inode *root_inode = sb_ret->s_op->alloc_inode(sb_ret, 2, 0, true);
	if(IS_ERR_PTR(root_inode)) {
		sb_ret = ERR_PTR(PTR_ERR(root_inode));
		goto exit;
	}
	root_inode->i_pinned = true;

	// 初始化根目录dentry
	qstr root_name = {.name = "/", .len = 1};
	dentry *root_dentry = sb_ret->s_op->alloc_dentry(sb_ret, root_inode, nullptr, root_name);
	if(IS_ERR_PTR(root_dentry)) {
		sb_ret = ERR_PTR(PTR_ERR(root_dentry));
		goto exit;
	}
	root_dentry->d_pinned = true;
	sb_ret->s_root = root_dentry;

	// 初始化根目录vfsmount
	vfsmount *mnt = &sb_ret->s_mnt;
	mnt->mnt_parent = nullptr;
	mnt->mnt_mntpoint = nullptr;
	mnt->mnt_sb = sb_ret;

exit:
	kfree(buf);
	return sb_ret;
}

inode* 	alloc_inode_ext2(super_block* sb, uint ino, uint size, bool isdir)
{
	inode *ret;
	Byte *buffer = (Byte*)kmalloc(SECTOR_SIZE);
	if (!buffer) {
		ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	ret = (inode*)kmalloc(sizeof(inode));
	if (!ret) {
		ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	ret->i_ino 		= ino + sb->s_basesec;
	ret->i_iop 		= &inode_op_ext2;
	ret->i_fop 		= &file_op_ext2;
	ret->i_size 	= size;
	ret->i_pinned 	= false;
	ret->i_sb 		= sb; 
	ret->i_count 	= 0;
	ret->i_isdir 	= isdir;
	INIT_LIST_HEAD(&ret->i_hash);
	INIT_LIST_HEAD(&ret->i_LRU);
	INIT_LIST_HEAD(&ret->i_dentry);

	// 找到组描述符所在的扇区
    uint group = (ino - 1) / sb->s_info_ext2->info_inode_pgroup;
	uint group_psec = SECTOR_SIZE / sizeof(group_ext2);
    uint sec = sb->s_info_ext2->info_1st_gdt_sec + group / group_psec;
    uint offset = group % group_psec * sizeof(group_ext2);
    if(!sd_read_block(buffer, sec, 1)) {
		ret = ERR_PTR(-ERROR_READ_SDCARD);
		goto exit;
	}
    uint group_base_blk = *(DWord*)(buffer + offset);
    uint group_base_sec = sb->s_basesec + group_base_blk * (sb->s_blocksize >> SECTOR_SHIFT);

    // 找到inode所在扇区，读取信息
	uint inode_psec = SECTOR_SIZE / sb->s_info_ext2->info_inode_size;
	uint off_sec = ( (ino - 1) % sb->s_info_ext2->info_inode_pgroup ) / inode_psec;
    uint data_sec = group_base_sec + 2 * (sb->s_blocksize >> SECTOR_SHIFT) + off_sec;
    if(!sd_read_block(buffer, data_sec, 1)) {
		ret = ERR_PTR(-ERROR_READ_SDCARD);
		goto exit;
	}
    uint off_bytes = ((ino - 1) % inode_psec) * sb->s_info_ext2->info_inode_size;
    inode_ext2* node_ext2 = (inode_ext2*)(buffer + off_bytes);
    ret->i_blocks 	= node_ext2->i_blocks;
    ret->i_size 	= node_ext2->i_size;

	// 构建块映射相关信息
	INIT_LIST_HEAD(&ret->i_data_map);
	for(int i=0; i<ret->i_blocks; i++) {
		fs_map* map = (fs_map*)kmalloc(sizeof(fs_map));
		if (map == nullptr) {
			ret = ERR_PTR(-ERROR_NO_MEMORY);
			goto exit;
		}
		map->map_page 	= nullptr;
		map->map_addr 	= find_page_sec_ext2(ret, i, (uint*)node_ext2->i_block_addr);

		list_add_tail(&map->map_node, &ret->i_data_map);
	}
	if(!add_to_cache(I_CACHE, ret))
		ret = ERR_PTR(-ERROR_CACHE_FULL);
	
exit:
	kfree(buffer);
	return ret;
}

dentry* alloc_dentry_ext2(super_block* sb, inode* node, dentry* parent, const qstr name)
{
	dentry *ret = (dentry*)kmalloc(sizeof(dentry));
	if (!ret) {
		ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	ret->d_name.name 	= name.name;
	ret->d_name.len 	= name.len;
	ret->d_op 			= &dentry_op_ext2;
	ret->d_pinned 		= false;
	ret->d_sb 			= sb;
	ret->d_count 		= 0;
	ret->d_mounted 		= false;
	ret->d_inode 		= node;
	ret->d_parent 		= parent? parent: ret;
	INIT_LIST_HEAD(&ret->d_hash);
	INIT_LIST_HEAD(&ret->d_LRU);
	INIT_LIST_HEAD(&ret->d_sibling);
	INIT_LIST_HEAD(&ret->d_subdirs);
	INIT_LIST_HEAD(&ret->d_alias);

	// 更新关联信息
	i_inc_count(node);
	list_add(&ret->d_alias, &node->i_dentry);
	if(parent) {
		list_add(&ret->d_sibling, &parent->d_subdirs);
	}
	if(!add_to_cache(D_CACHE, ret))
		ret = ERR_PTR(-ERROR_CACHE_FULL);

exit:
	return ret;
}

dentry* create_dentry_ext2(super_block* sb, dentry* parent, const qstr name, bool isdir)
{
	return ERR_PTR(ERROR_NOT_IMPLEMENT);
}


dentry* lookup_ext2(dentry* parent_entry, const qstr name)
{
	super_block* sb = parent_entry->d_sb;
	inode* parent_node = parent_entry->d_inode;
	dentry* ret_entry = ERR_PTR(-ERROR_FILE_NOTFOUND);

	list_head *p;
	list_for_each(p, &parent_node->i_data_map) {
		fspage* cur_page = get_fspage(p, sb->s_blocksize);
		if(IS_ERR_PTR(cur_page)) {
			ret_entry = ERR_PTR(PTR_ERR(cur_page));
			goto exit;
		}

		Byte *cur_entry = cur_page->p_data;
		dentry_ext2* entry_ext2;
		for( ; (cur_entry - cur_page->p_data) < sb->s_blocksize; cur_entry += entry_ext2->d_size) {
			entry_ext2 = (dentry_ext2*)cur_entry;

			// 比较文件名
			qstr cur_name = {
				.name 	= entry_ext2->d_name,
				.len 	= entry_ext2->d_name_len
			};
			if(parent_entry->d_op->compare(name, cur_name) == 0) {
				inode* node = alloc_inode_ext2(sb, entry_ext2->d_ino, entry_ext2->d_size, entry_ext2->d_type == EXT2_FILE_TYPE_DIR);
				ret_entry = alloc_dentry_ext2(sb, node, parent_entry, name);
				goto exit;
			}
		}
	}

exit:
	return ret_entry;
}

int expand_ext2(inode* node, int num)
{
	return ERROR_NOT_IMPLEMENT;
}

int clear_ext2(inode* node, dentry* parent, const qstr name)
{
	return ERROR_NOT_IMPLEMENT;
}

int delete_ext2(inode* node, dentry* parent, const qstr name)
{
	return ERROR_NOT_IMPLEMENT;
}


int compare_ext2(const qstr a, const qstr b)
{
	// ext2文件名区分大小写
	const char *p = a.name;
	const char *q = b.name;
	int pos = 0;
	for(pos = 0; pos < a.len && pos < b.len; pos++) {
		if(*p && (*p == *q) ) {
			p ++;
			q ++;
		} else {
			break;
		}
	}

	char cur_a = (pos == a.len? '\0': *p);
	char cur_b = (pos == b.len? '\0': *q);

	return cur_a - cur_b;
}

dentry* list_dir_ext2(file* file)
{
	dentry *entry = file->f_dentry;
	inode *node = entry->d_inode;
	super_block *sb = entry->d_sb;

	list_head *p;
	list_for_each(p, &node->i_data_map) {
		fspage* cur_page = get_fspage(p, sb->s_blocksize);
		if(IS_ERR_PTR(cur_page)) {
			entry = ERR_PTR(PTR_ERR(cur_page));
			goto exit;
		}

		Byte *cur_entry = cur_page->p_data;
		dentry_ext2* entry_ext2;
		for( ; (cur_entry - cur_page->p_data) < sb->s_blocksize; cur_entry += entry_ext2->d_size) {
			entry_ext2 = (dentry_ext2*)cur_entry;

			// 构建该目录下所有文件的dentry，并加入subdir中
			inode* node = alloc_inode_ext2(sb, entry_ext2->d_ino, entry_ext2->d_size, entry_ext2->d_type == EXT2_FILE_TYPE_DIR);
			qstr cur_name = {
				.name 	= entry_ext2->d_name,
				.len 	= entry_ext2->d_name_len
			};
			dentry *cur_entry = alloc_dentry_ext2(sb, node, entry, cur_name);
			cur_entry->d_inode->i_pinned = true;
			cur_entry->d_pinned = true;
			if(entry_ext2->d_type == EXT2_FILE_TYPE_DIR)
				cur_entry->d_inode->i_isdir = true;
			d_inc_count(cur_entry);
		}
	}

exit:
	return entry;
}

int close_ext2(file* file)
{
	return NO_ERROR;
}

uint find_page_sec_ext2(inode *node, int curPage, uint* pageaddr) {
	uint ret;
	super_block *sb = node->i_sb;
	uint addr = (uint)pageaddr;

	Byte *data = (Byte*)kmalloc(sb->s_blocksize);
    if (!data) {
        ret = -ERROR_NO_MEMORY;
		goto exit;
	}

	// 直接映射  
    if(curPage < 12) {
        ret = pageaddr[curPage];
		goto exit;
	} 
  
    int num = sb->s_blocksize >> 2; 	// 每块4字节
    // 一级索引 
    curPage -= 12;
    if(curPage < num) {  
        sd_read_block(data, pageaddr[12], sb->s_blocksize >> SECTOR_SHIFT);
        ret = *(uint*)(data + (curPage << 2));
        goto exit;
    }  
  
    // 二级索引  
    curPage -= num;  
    if ( curPage < num * num ) {
        sd_read_block(data, pageaddr[13], sb->s_blocksize >> SECTOR_SHIFT);
        addr = *(uint*)(data + ((curPage / num) << 2) );
        sd_read_block(data, addr, sb->s_blocksize >> SECTOR_SHIFT);
        ret = *(uint*)(data + ((curPage % num) << 2) );
        goto exit;
    }

    // 三级索引 
    curPage -= num * num; 
    if ( curPage < num * num * num ) {
        sd_read_block(data, pageaddr[14], sb->s_blocksize >> SECTOR_SHIFT);
        addr = *(uint*)(data + ((curPage / (num * num)) << 2) );
        sd_read_block(data, addr, sb->s_blocksize >> SECTOR_SHIFT);
        addr = *(uint*)(data + ((curPage / (num * num) / num) << 2) );
        sd_read_block(data, addr, sb->s_blocksize >> SECTOR_SHIFT);
        ret = *(uint*)(data + ((curPage / (num * num) % num) << 2) );
        goto exit;
    }

exit:
    kfree(data);
    return ret;
};