#include "fat.h"

#include <driver/sd.h>
#include <driver/vga.h>

#include <ouros/error.h>
#include <ouros/fs/fscache.h>

static file_system_type fs_type_fat32 = {
	.name 				= 		"fat32",
	.get_sb 			= 		get_sb_fat32,
};

static super_block_op sb_op_fat32 = {
	.alloc_inode 		=		alloc_inode_fat32,
	.alloc_dentry 		=		alloc_dentry_fat32,
	.create_dentry 		= 		create_dentry_fat32,
};

static inode_op inode_op_fat32 = {
	.lookup 			= 		lookup_fat32,
	.expand 			= 		expand_fat32,
	.clear 				= 		clear_fat32,
	.delete 			= 		delete_fat32,
};

static dentry_op dentry_op_fat32 = {
	.compare 			= 		compare_fat32,
	.open 				= 		dentry_open_file,
};

static file_op file_op_fat32 = {
	.list_dir 			= 		list_dir_fat32,
	.close 				= 		close_fat32,
};

file_system_type* get_fs_type_fat32()
{
	return &fs_type_fat32;
}

super_block* get_sb_fat32(DWord base_sec)
{
	super_block* sb_ret = nullptr;

	// 申请缓存区读取扇区数据
	Byte *buf = (Byte*)kmalloc_fake(SECTOR_SIZE);
	if (buf == nullptr) {
		sb_ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	
	// 初始化FAT32系统有关信息
	fs_info_fat32* fs_info = (fs_info_fat32*)kmalloc_fake(sizeof(fs_info_fat32));
	// 从SD卡读取FSINFO扇区信息
	if (!sd_read_block(buf, base_sec + 1, 1)) {
		sb_ret = ERR_PTR(-ERROR_READ_SDCARD);
		goto exit;
	}
	fs_info->info_next_clu 	= READ_FSINFO_NEXT_CLU(buf);
	// 从SD卡读取DBR头部信息
	if (!sd_read_block(buf, base_sec, 1)) {
		sb_ret = ERR_PTR(-ERROR_READ_SDCARD);
		goto exit;
	}
	dbr_head *dbr = (dbr_head*)buf;
	fs_info->info_fsinfo_sec= base_sec + 1;
	fs_info->info_fat_sec 	= base_sec + dbr->dbr_reserved_num;
	fs_info->info_clu_base 	= fs_info->info_fat_sec + (dbr->dbr_fat_num * dbr->dbr_fat_secsize);
	if(fs_info->info_next_clu == 0xFFFFFFFF)
		fs_info->info_next_clu = dbr->dbr_root_clu;

	// 构建超级块super_block
	sb_ret = (super_block*)kmalloc_fake(sizeof(super_block));
	if (sb_ret == nullptr) {
		sb_ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	sb_ret->s_op 			= &sb_op_fat32;
	sb_ret->s_type 			= &fs_type_fat32;
	sb_ret->s_blocksize 	= dbr->dbr_sec_per_clu << SECTOR_SHIFT;
	sb_ret->s_blockbits 	= SECTOR_SHIFT;
	sb_ret->s_info_fat32 	= fs_info;
	sb_ret->s_basesec 		= base_sec;
	Byte temp = dbr->dbr_sec_per_clu;
	while(temp >>= 1) {
		sb_ret->s_blockbits ++;
	}
	INIT_LIST_HEAD(&sb_ret->s_listnode);

	// 初始化根目录inode
	inode *root_inode = sb_ret->s_op->alloc_inode(sb_ret, dbr->dbr_root_clu, 0, true);
	if(IS_ERR_PTR(root_inode)) {
		sb_ret = ERR_PTR(PTR_ERR(root_inode));
		goto exit;
	}
	root_inode->i_pinned = true;
	// 预读取根目录内容，以确定根目录文件大小
	list_head *p;
	list_for_each(p, &root_inode->i_data_map) {
		fspage* cur_page = get_fspage(p, sb_ret->s_blocksize);
		if(IS_ERR_PTR(cur_page)) {
			sb_ret = ERR_PTR(PTR_ERR(cur_page));
			goto exit;
		}

		sfn_entry* sfn_item = (sfn_entry*)cur_page->p_data;
		for( ; ( (Byte*)sfn_item - cur_page->p_data) < sb_ret->s_blocksize; sfn_item ++) {
			// 判断标志位和文件属性
			// kernel_printf("mark: 0x%X, attr: 0x%X\n", sfn_item->sfn_mark, sfn_item->sfn_attr);
			if(sfn_item->sfn_mark == SFN_MARK_UNUSED) {
				break;
			} else {
				root_inode->i_size += sizeof(sfn_entry);
			}
		}
	}

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
	kfree_fake(buf);
	return sb_ret;
}

DWord read_fat(DWord fat_base_sec, DWord clu_num)
{
	DWord off_bytes = clu_num << 2;
	DWord off_sec = off_bytes >> SECTOR_SHIFT;

	DWord ret;
	Byte *buf = (Byte*)kmalloc_fake(SECTOR_SIZE);
	if (buf == nullptr) {
		ret = -ERROR_NO_MEMORY;
	} else if (!sd_read_block(buf, fat_base_sec + off_sec, 1)) {
		ret = -ERROR_READ_SDCARD;
	} else {
		ret = *(DWord*) (&buf[off_bytes & (SECTOR_SIZE-1)]);
	}
	return ret;
}

int write_fat(DWord fat_base_sec, DWord clu_num, DWord value)
{
	DWord off_bytes = clu_num << 2;
	DWord off_sec = off_bytes >> SECTOR_SHIFT;

	int error = NO_ERROR;
	Byte *buf = (Byte*)kmalloc_fake(SECTOR_SIZE);
	if (buf == nullptr) {
		error = -ERROR_NO_MEMORY;
	} else if (!sd_read_block(buf, fat_base_sec + off_sec, 1)) {
		error = -ERROR_READ_SDCARD;
	} else {
		*(DWord*) (&buf[off_bytes & (SECTOR_SIZE-1)]) = value;
		if (!sd_write_block(buf, fat_base_sec + off_sec, 1)) {
			error = -ERROR_READ_SDCARD;
		}
	}
	return error;
}

DWord find_free_clu(DWord fat_base_sec, DWord start_idx)
{
	DWord free_idx = start_idx - 1;
	uint fat_value = ~FAT_ITEM_FREE;
	while(fat_value != FAT_ITEM_FREE) {
		fat_value = read_fat(fat_base_sec, ++free_idx);
		if(IS_ERR_VALUE(fat_value)) {
			free_idx = fat_value;
			goto exit;
		}
	}
exit:
	return free_idx;
}

int update_next_clu(super_block *sb, DWord value)
{
	int error = NO_ERROR;
	Byte *buf = (Byte*)kmalloc_fake(SECTOR_SIZE);
	if(!buf) {
		error = -ERROR_NO_MEMORY;
		goto exit;
	}
	if (!sd_read_block(buf, sb->s_info_fat32->info_fsinfo_sec, 1)) {
		error = -ERROR_READ_SDCARD;
		goto exit;
	}
	sb->s_info_fat32->info_next_clu = value;
	WRITE_FSINFO_NEXT_CLU(buf, value);
	if (!sd_write_block(buf, sb->s_info_fat32->info_fsinfo_sec, 1)) {
		error = -ERROR_READ_SDCARD;
		goto exit;
	}
exit:
	return error;
}

void convert_normal_to_8dot3(const qstr* src, struct filename_8dot3* dst)
{
	// kernel_printf("before: %s\n", src->name);
	// 找到'.'的位置
	int dot_pos = -1;
	for(int i = src->len-1; i >= 0; i --) {
		if(src->name[i] == '.') {
			dot_pos = i;
			break;
		}
	}
	if(dot_pos < 0)
		dot_pos = src->len;

	// 填写对应的名字，未填满的地方填空格
	for(int i=0; i<8; i++) {
		dst->name[i] = i < dot_pos? TO_UPPER_CASE(src->name[i]): DUMMY_CHAR_8DOT3;
	}
	for(int i=0; i<3; i++) {
		int pos = dot_pos + 1 + i;
		dst->ext[i] = pos < src->len? TO_UPPER_CASE(src->name[pos]) : DUMMY_CHAR_8DOT3;
	}
	// kernel_printf("after: %s.%s\n", dst->name, dst->ext);
}

void convert_8dot3_to_normal(const struct filename_8dot3* src, qstr* dst)
{
	// 申请字符串内存，大小为8字节名称 + 3字节扩展名 + 1字节'.' + 1字节'\0'
	// kernel_printf("before: %s.%s\n", src->name, src->ext);
	char* name = (char*)kmalloc_fake(8 + 3 + 1 + 1);
	int len = 0;

	// 填写对应的名字，遇到空格结束
	for(int i=0; i<8; i++) {
		if(src->name[i] == DUMMY_CHAR_8DOT3) {
			break;
		}
		name[len++] = TO_LOWER_CASE(src->name[i]);
	}
	for(int i=0; i<3; i++) {
		if(src->ext[i] == DUMMY_CHAR_8DOT3) {
			break;
		} else if(i == 0) {
			name[len ++] = '.';
		}
		name[len++] = TO_LOWER_CASE(src->ext[i]);
	}
	name[len] = '\0';

	dst->name = name;
	dst->len = len;

	// kernel_printf("after: %s\n", dst->name);
}

dentry* read_dentry_from_sfn(sfn_entry* sfn_item, dentry *parent_entry, qstr name)
{
	dentry *ret_entry;
	super_block *sb = parent_entry->d_sb;

	// 获得找到的inode
	uint first_clu = sfn_item->sfn_1stclu_low + (sfn_item->sfn_1stclu_high << 16);
	inode* new_inode = lookup_cache(I_CACHE, CLU_TO_SEC(sb->s_info_fat32->info_clu_base, first_clu, sb->s_blockbits));
	if(!new_inode){
		new_inode = sb->s_op->alloc_inode(sb, first_clu, 
			sfn_item->sfn_file_size, sfn_item->sfn_attr == SFN_ATTR_DIR);
	}
	
	if(IS_ERR_PTR(new_inode)) {
		ret_entry = ERR_PTR(PTR_ERR(new_inode));
		goto exit;
	}
	// 获得找到的dentry
	ret_entry = sb->s_op->alloc_dentry(sb, new_inode, parent_entry, name);
	if(IS_ERR_PTR(ret_entry)) {
		goto exit;
	}
	// 如果发现dentry名为".."，需要去父目录找它的名字
	qstr dotdot = {.name = "..", .len = 2};
	if(compare_fat32(ret_entry->d_name, dotdot) == 0) {
		kernel_printf("find parent directory name!!\n");

		file *cur = dentry_open_file(ret_entry, F_MODE_READ);
		sfn_entry sfn;
		while(!eof(cur)) {
			fs_read(cur, &sfn, sizeof(sfn));
			uint clu_num = sfn.sfn_1stclu_low + (sfn.sfn_1stclu_high << 16);
			if(clu_num == first_clu) {
				convert_8dot3_to_normal(&sfn.sfn_filename, &ret_entry->d_name);
				break;
			}
		}
		fs_close(cur);

		if(compare_fat32(ret_entry->d_name, dotdot) == 0) {
			ret_entry = ERR_PTR(-ERROR_FILE_NOTFOUND);
		}
	}
exit:
	return ret_entry;
}

inode* alloc_inode_fat32(super_block* sb, uint base_blk_addr, uint size, bool isdir)
{
	inode *ret = (inode*)kmalloc_fake(sizeof(inode));
	if (!ret) {
		ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	ret->i_ino 		= CLU_TO_SEC(sb->s_info_fat32->info_clu_base, base_blk_addr, sb->s_blockbits);
	ret->i_iop 		= &inode_op_fat32;
	ret->i_fop 		= &file_op_fat32;
	ret->i_size 	= size;
	ret->i_pinned 	= false;
	ret->i_sb 		= sb; 
	ret->i_count 	= 0;
	ret->i_isdir 	= isdir;
	INIT_LIST_HEAD(&ret->i_hash);
	INIT_LIST_HEAD(&ret->i_LRU);
	INIT_LIST_HEAD(&ret->i_dentry);

	// 构建块映射相关信息
	INIT_LIST_HEAD(&ret->i_data_map);
	ret->i_blocks 	= 0;
	uint clu_num = base_blk_addr;
	while(clu_num != FAT_ITEM_END) {
		fs_map* map = (fs_map*)kmalloc_fake(sizeof(fs_map));
		if (map == nullptr) {
			ret = ERR_PTR(-ERROR_NO_MEMORY);
			goto exit;
		}
		ret->i_blocks ++;
		map->map_page 	= nullptr;
		map->map_addr 	= CLU_TO_SEC(sb->s_info_fat32->info_clu_base, clu_num, sb->s_blockbits);
		list_add_tail(&map->map_node, &ret->i_data_map);
		
		clu_num = read_fat(sb->s_info_fat32->info_fat_sec, clu_num);
		if(IS_ERR_VALUE(clu_num)) {
			ret = ERR_PTR(clu_num);
			goto exit;
		}
	}
	if(!add_to_cache(I_CACHE, ret))
		ret = ERR_PTR(-ERROR_CACHE_FULL);
	
exit:
	return ret;
}

dentry* alloc_dentry_fat32(super_block* sb, inode* node, dentry* parent, const qstr name)
{
	dentry *ret = (dentry*)kmalloc_fake(sizeof(dentry));
	if (!ret) {
		ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	ret->d_name.name 	= name.name;
	ret->d_name.len 	= name.len;
	ret->d_op 			= &dentry_op_fat32;
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

dentry* create_dentry_fat32(super_block* sb, dentry* parent, const qstr name, bool isdir)
{
	dentry *entry_ret = nullptr;
	// 找到空闲簇
	uint free_clu_idx = sb->s_info_fat32->info_next_clu;
	free_clu_idx = find_free_clu(sb->s_info_fat32->info_fat_sec, free_clu_idx);
	// 将该簇写为END标志
	int error = write_fat(sb->s_info_fat32->info_fat_sec, free_clu_idx, FAT_ITEM_END);
	if(IS_ERR_VALUE(error)) {
		entry_ret = ERR_PTR(error);
		goto exit;
	}

	// 构建inode
	inode *node = sb->s_op->alloc_inode(sb, free_clu_idx, 0, isdir);
	if(IS_ERR_PTR(node)) {
		entry_ret = ERR_PTR(PTR_ERR(node));
		goto exit;
	}
	// 构建dentry
	entry_ret = sb->s_op->alloc_dentry(sb, node, parent, name);

	// 在父目录inode中加入目录项信息
	file *parent_file = dentry_open_file(parent, F_MODE_APPEND);
	sfn_entry sfn;
	convert_normal_to_8dot3(&name, &sfn.sfn_filename);
	sfn.sfn_attr 		= isdir? SFN_ATTR_DIR: SFN_ATTR_WRITE;
	sfn.sfn_reserved 	= 0x08;
	sfn.sfn_1stclu_high = free_clu_idx >> 16;
	sfn.sfn_1stclu_low 	= free_clu_idx & 0xFFFF;
	sfn.sfn_file_size	= 0;
	fs_write(parent_file, (Byte*)&sfn, sizeof(sfn));
	fs_close(parent_file);

exit:
	return entry_ret;
}

dentry* lookup_fat32(dentry* parent_entry, const qstr name)
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

		sfn_entry* sfn_item = (sfn_entry*)cur_page->p_data;
		for( ; ( (Byte*)sfn_item - cur_page->p_data) < sb->s_blocksize; sfn_item ++) {
			// 判断标志位和文件属性
			// kernel_printf("mark: 0x%X, attr: 0x%X\n", sfn_item->sfn_mark, sfn_item->sfn_attr);
			if(sfn_item->sfn_mark == SFN_MARK_UNUSED) {
				break;
			} else if(sfn_item->sfn_mark == SFN_MARK_DELETED) {
				continue;
			} else if(sfn_item->sfn_attr == SFN_ATTR_LABEL || sfn_item->sfn_attr == SFN_ATTR_LFN) {
				continue;
			}

			// 比较文件名
			qstr cur_name;
			convert_8dot3_to_normal(&sfn_item->sfn_filename, &cur_name);
			if(parent_entry->d_op->compare(name, cur_name) == 0) {
				ret_entry = read_dentry_from_sfn(sfn_item, parent_entry, name);
				goto exit;
			}
		}
	}

exit:
	return ret_entry;
}

int expand_fat32(inode* node, int num)
{
	i_inc_count(node);
	int error = NO_ERROR;

	super_block *sb = node->i_sb;
	uint free_clu_idx = sb->s_info_fat32->info_next_clu;
	uint last_clu_idx = SEC_TO_CLU(sb->s_info_fat32->info_clu_base, node->i_ino, sb->s_blockbits);
	for(int i=0; i<num; i++)
	{
		// 找到空闲簇
		free_clu_idx = find_free_clu(sb->s_info_fat32->info_fat_sec, free_clu_idx);
		// 将原inode最后一簇的下一簇置为该簇
		error = write_fat(sb->s_info_fat32->info_fat_sec, last_clu_idx, free_clu_idx);
		if(IS_ERR_VALUE(error)) {
			goto exit;
		}
		last_clu_idx = free_clu_idx;

		// 更新inode信息
		fs_map* map = (fs_map*)kmalloc_fake(sizeof(fs_map));
		if (map == nullptr) {
			error = -ERROR_NO_MEMORY;
			goto exit;
		}
		node->i_blocks ++;
		map->map_page 	= nullptr;
		map->map_addr 	= CLU_TO_SEC(sb->s_info_fat32->info_clu_base, free_clu_idx, sb->s_blockbits);
		list_add_tail(&map->map_node, &node->i_data_map);

		// 现在这一簇肯定不是空闲的，因此跳到下一簇
		free_clu_idx ++;
	}

	// 将最后一簇的下一簇置为END标志
	error = write_fat(sb->s_info_fat32->info_fat_sec, last_clu_idx, FAT_ITEM_END);
	if(IS_ERR_VALUE(error)) {
		goto exit;
	}
	// 更新外存中的寻找下一个可用簇信息
	update_next_clu(sb, free_clu_idx);

exit:
	i_dec_count(node);
	return error;
}

int clear_fat32(inode* node, dentry* parent, const qstr name)
{
	i_inc_count(node);
	int error = NO_ERROR;

	super_block *sb = node->i_sb;
	uint clu_num = SEC_TO_CLU(sb->s_info_fat32->info_clu_base, node->i_ino, sb->s_blockbits);
	uint next_clu = read_fat(sb->s_info_fat32->info_fat_sec, clu_num);
	uint min_clu = 0xFFFFFFFF;
	// 先把当前簇号的值写为END标记
	if(next_clu != FAT_ITEM_END) {
		error = write_fat(sb->s_info_fat32->info_fat_sec, clu_num, FAT_ITEM_END);
		if(IS_ERR_VALUE(error)) {
			goto exit;
		}
	}
	// 把之后所有簇号的值写为FREE标记
	while(next_clu != FAT_ITEM_END) {
		// 读取下一个簇号
		clu_num = next_clu;
		next_clu = read_fat(sb->s_info_fat32->info_fat_sec, clu_num);
		if(IS_ERR_VALUE(next_clu)) {
			error = next_clu;
			goto exit;
		}
		// 将当前簇号的值写为FREE标记
		error = write_fat(sb->s_info_fat32->info_fat_sec, clu_num, FAT_ITEM_FREE);
		if(IS_ERR_VALUE(error)) {
			goto exit;
		}
		if(clu_num < min_clu) {
			min_clu = clu_num;
		}
	}
	// 更新外存中的寻找下一个可用簇信息
	if(min_clu < sb->s_info_fat32->info_next_clu) {
		update_next_clu(sb, min_clu);
	}

	// 找到该inode的父目录项并更改信息
	inode *parent_node = parent->d_inode;
	list_head *p;
	list_for_each(p, &parent_node->i_data_map) {
		fspage* cur_page = get_fspage(p, sb->s_blocksize);
		if(IS_ERR_PTR(cur_page)) {
			error = PTR_ERR(cur_page);
			goto exit;
		}

		sfn_entry* sfn_item = (sfn_entry*)cur_page->p_data;
		for( ; ( (Byte*)sfn_item - cur_page->p_data) < sb->s_blocksize; sfn_item ++) {
			// kernel_printf("mark: 0x%X, attr: 0x%X\n", sfn_item->sfn_mark, sfn_item->sfn_attr);
			
			// 判断标志位和文件属性
			if(sfn_item->sfn_mark == SFN_MARK_UNUSED) {
				break;
			} else if(sfn_item->sfn_mark == SFN_MARK_DELETED) {
				continue;
			} else if((sfn_item->sfn_attr == SFN_ATTR_LFN) 
					|| (sfn_item->sfn_attr & SFN_ATTR_LABEL) 
					|| (sfn_item->sfn_attr & SFN_ATTR_SYSTEM)) {
				continue;
			}

			// 找到该文件的目录项
			qstr cur_name;
			convert_8dot3_to_normal(&sfn_item->sfn_filename, &cur_name);
			if(compare_fat32(cur_name, name) == 0) {
				sfn_item->sfn_file_size = 0;
				cur_page->p_dirt = true;
				error = flush_fspage(cur_page);
				goto modify_node;
			}
		}
	}

modify_node:
	// 更新node信息
	node->i_blocks 	= 1;
	node->i_size 	= 0;
	list_head *remained = node->i_data_map.next;
	list_for_each(p, remained) {
		if(p == &node->i_data_map)
			break;
		list_head *cur = p;
		p = p->prev;
		list_del(cur);
		fs_map *map = container_of(cur, fs_map, map_node);
		kfree_fake(map);
	}

exit:
	i_dec_count(node);
	return error;
}

int delete_fat32(inode* node, dentry* parent, const qstr name)
{
	int error = NO_ERROR;
	super_block *sb = node->i_sb;
	uint clu_num = SEC_TO_CLU(sb->s_info_fat32->info_clu_base, node->i_ino, sb->s_blockbits);
	uint min_clu = 0xFFFFFFFF;
	uint next_clu;
	do {
		// 读取下一个簇号
		next_clu = read_fat(sb->s_info_fat32->info_fat_sec, clu_num);
		if(IS_ERR_VALUE(next_clu)) {
			error = next_clu;
			goto exit;
		}
		// 将当前簇号的值写为FREE标记
		error = write_fat(sb->s_info_fat32->info_fat_sec, clu_num, FAT_ITEM_FREE);
		if(IS_ERR_VALUE(error)) {
			goto exit;
		}
		if(clu_num < min_clu) {
			min_clu = clu_num;
		}
		clu_num = next_clu;
	} while(next_clu != FAT_ITEM_END);
	// 更新外存中的寻找下一个可用簇信息
	if(min_clu < sb->s_info_fat32->info_next_clu) {
		update_next_clu(sb, min_clu);
	}

	// 找到该inode的父目录项并更改信息
	inode *parent_node = parent->d_inode;
	list_head *p;
	list_for_each(p, &parent_node->i_data_map) {
		fspage* cur_page = get_fspage(p, sb->s_blocksize);
		if(IS_ERR_PTR(cur_page)) {
			error = PTR_ERR(cur_page);
			goto exit;
		}

		sfn_entry* sfn_item = (sfn_entry*)cur_page->p_data;
		for( ; ( (Byte*)sfn_item - cur_page->p_data) < sb->s_blocksize; sfn_item ++) {
			// kernel_printf("mark: 0x%X, attr: 0x%X\n", sfn_item->sfn_mark, sfn_item->sfn_attr);
			
			// 判断标志位和文件属性
			if(sfn_item->sfn_mark == SFN_MARK_UNUSED) {
				break;
			} else if(sfn_item->sfn_mark == SFN_MARK_DELETED) {
				continue;
			} else if((sfn_item->sfn_attr == SFN_ATTR_LFN) 
					|| (sfn_item->sfn_attr & SFN_ATTR_LABEL) 
					|| (sfn_item->sfn_attr & SFN_ATTR_SYSTEM)) {
				continue;
			}

			// 找到该文件的目录项
			qstr cur_name;
			convert_8dot3_to_normal(&sfn_item->sfn_filename, &cur_name);
			if(compare_fat32(cur_name, name) == 0) {
				sfn_item->sfn_mark = SFN_MARK_DELETED;
				cur_page->p_dirt = true;
				error = flush_fspage(cur_page);
				goto modify_node;
			}
		}
	}

modify_node:
	// 在缓存中删除inode
	free_inode(node);

exit:
	return error;
}

int compare_fat32(const qstr a, const qstr b)
{
	// fat32文件名不区分大小写
	const char *p = a.name;
	const char *q = b.name;
	int pos = 0;
	for(pos = 0; pos < a.len && pos < b.len; pos++) {
		if(*p && (TO_UPPER_CASE(*p) == TO_UPPER_CASE(*q)) ) {
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

dentry* list_dir_fat32(file* file)
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

		sfn_entry* sfn_item = (sfn_entry*)cur_page->p_data;
		for( ; ( (Byte*)sfn_item - cur_page->p_data) < sb->s_blocksize; sfn_item ++) {
			// kernel_printf("mark: 0x%X, attr: 0x%X\n", sfn_item->sfn_mark, sfn_item->sfn_attr);
			
			// 判断标志位和文件属性
			if(sfn_item->sfn_mark == SFN_MARK_UNUSED) {
				break;
			} else if(sfn_item->sfn_mark == SFN_MARK_DELETED) {
				continue;
			} else if((sfn_item->sfn_attr == SFN_ATTR_LFN) 
					|| (sfn_item->sfn_attr & SFN_ATTR_LABEL) 
					|| (sfn_item->sfn_attr & SFN_ATTR_SYSTEM)) {
				continue;
			}

			// 构建该目录下所有文件的dentry，并加入subdir中
			qstr cur_name;
			convert_8dot3_to_normal(&sfn_item->sfn_filename, &cur_name);
			// 跳过.和..
			qstr dot = {.name = ".", .len = 1};
			qstr dotdot = {.name = "..", .len = 2};
			if(compare_fat32(cur_name, dot) == 0 || compare_fat32(cur_name, dotdot) == 0)
				continue;

			nameidata nd;
			nd.nd_dentry 		= entry;
			nd.nd_cur_name.name = cur_name.name;
			nd.nd_cur_name.len 	= cur_name.len;
			nd.nd_mnt 			= &sb->s_mnt;
			dentry *cur_entry;
			// kernel_printf("clu: %d\n", sfn_item->sfn_1stclu_low + (sfn_item->sfn_1stclu_high << 16));
			if(!(cur_entry = lookup_cache(D_CACHE, &nd))) {
				cur_entry = read_dentry_from_sfn(sfn_item, entry, cur_name);
			}
			cur_entry->d_inode->i_pinned = true;
			cur_entry->d_pinned = true;
			if(sfn_item->sfn_attr == SFN_ATTR_DIR)
				cur_entry->d_inode->i_isdir = true;
			d_inc_count(cur_entry);
		}
	}

exit:
	return entry;
}

int close_fat32(file* file)
{
	int error = -ERROR_FILE_NOTFOUND;
	dentry *entry = get_dentry_parent(file->f_dentry);
	if(entry == file->f_dentry)
		goto exit;
	inode *node = entry->d_inode;
	super_block *sb = entry->d_sb;

	list_head *p;
	list_for_each(p, &node->i_data_map) {
		fspage* cur_page = get_fspage(p, sb->s_blocksize);
		if(IS_ERR_PTR(cur_page)) {
			error = PTR_ERR(cur_page);
			goto exit;
		}

		sfn_entry* sfn_item = (sfn_entry*)cur_page->p_data;
		for( ; ( (Byte*)sfn_item - cur_page->p_data) < sb->s_blocksize; sfn_item ++) {
			// kernel_printf("mark: 0x%X, attr: 0x%X\n", sfn_item->sfn_mark, sfn_item->sfn_attr);
			
			// 判断标志位和文件属性
			if(sfn_item->sfn_mark == SFN_MARK_UNUSED) {
				break;
			} else if(sfn_item->sfn_mark == SFN_MARK_DELETED) {
				continue;
			} else if((sfn_item->sfn_attr == SFN_ATTR_LFN) 
					|| (sfn_item->sfn_attr & SFN_ATTR_LABEL) 
					|| (sfn_item->sfn_attr & SFN_ATTR_SYSTEM)) {
				continue;
			}

			// 找到该文件的目录项
			qstr cur_name;
			convert_8dot3_to_normal(&sfn_item->sfn_filename, &cur_name);
			if(compare_fat32(cur_name, file->f_dentry->d_name) == 0) {
				sfn_item->sfn_file_size = file->f_dentry->d_inode->i_size;
				cur_page->p_dirt = true;
				error = flush_fspage(cur_page);
				goto exit;
			}
		}
	}
exit:
	return error;
}