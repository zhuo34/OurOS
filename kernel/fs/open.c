#include "open.h"

#include <ouros/error.h>
#include <ouros/utils.h>
#include <ouros/fs/fscache.h>

#include <driver/vga.h>

file* fs_open(const char *filename, uint mode)
{
	// 分析文件打开模式
	int open_flags = OPEN_FIND;
	if(mode & F_MODE_WRITE_MASK)
		open_flags |= OPEN_CREATE;
	if(mode & F_MODE_DIR_MASK)
		open_flags |= OPEN_DIR;

	// 寻找目标dentry
	nameidata nd;
	int error = path_lookup(filename, open_flags, &nd);
	
	// 根据dentry构建file结构
	file *ret;
	if(IS_ERR_VALUE(error)) {
		ret = ERR_PTR(error);
	} else {
		ret = nd.nd_dentry->d_op->open(nd.nd_dentry, mode);
	}
	return ret;
}

int fs_close(file *fp)
{
	int error = NO_ERROR;
	if(fp->f_dirt) {
		// 将数据写回外存
		fs_map *data_map = container_of(fp->f_data_map, fs_map, map_node);
		list_head *p;
		list_for_each(p, &data_map->map_node) {
			fs_map *map = container_of(p, fs_map, map_node);
			error = flush_fspage(map->map_page);
			if(IS_ERR_VALUE(error))
				goto exit;
		}
		// 修改父目录中的目录项信息
		error = fp->f_op->close(fp);
		if(IS_ERR_VALUE(error))
			goto exit;
	}

	// 释放file空间
	d_dec_count(fp->f_dentry);
	kfree_fake(fp);
exit:
	return error;
}

int path_lookup(const char *filename, int flags, nameidata *nd)
{
	dentry* entry = (*filename == '/')? get_root_dentry(): get_pwd_dentry();
	d_inc_count(entry);
	nd->nd_dentry = entry;
	nd->nd_mnt = &entry->d_sb->s_mnt;

	int error = link_path_walk(filename, nd);
	if(error == -ERROR_FILE_NOTFOUND
		&& (flags & OPEN_CREATE)
		&& qstr_endwith(filename, nd->nd_cur_name))
	{
		// 创建新的索引结点并构建对应的目录项
		super_block *sb = nd->nd_dentry->d_sb;
		bool isdir = flags & OPEN_DIR;
		entry = sb->s_op->create_dentry(sb, nd->nd_dentry, nd->nd_cur_name, isdir);
		error = IS_ERR_PTR(entry)? PTR_ERR(entry): NO_ERROR;
		if(IS_ERR_VALUE(error)) {
			goto exit;
		}
		// 如果是目录，清空该结点
		if(isdir) {
			list_head *p;
			list_for_each(p, &entry->d_inode->i_data_map) {
				fspage *page = get_fspage(p, sb->s_blocksize);
				kernel_memset(page->p_data, 0, sb->s_blocksize);
				page->p_dirt = true;
				flush_fspage(page);
			}
		}
		
		d_dec_count(nd->nd_dentry);
		d_inc_count(entry);
		nd->nd_dentry = entry;
		nd->nd_cur_name.name = "";
		nd->nd_cur_name.len  = 0;
	}
exit:
	d_dec_count(entry);
	return error;
}

int link_path_walk(const char *filename, nameidata *nd)
{
	int error = NO_ERROR;
	char c;
	while(true) {
		// 跳过多余的'/'和空白
		while(*filename == '/' || *filename == ' ' || *filename == '\t' || *filename == '\n')
			filename ++;
		// 如果文件名没有内容了，说明解析结束
		if(!*filename)
			break;
		
		// 获得当前分量的文件名
		nd->nd_cur_name.name = filename;
		do {
			c = *++filename;
		} while(c && c != '/');
		nd->nd_cur_name.len = filename - nd->nd_cur_name.name;
		// kernel_printf("now filename: %s, len = %d\n", nd->nd_cur_name.name, nd->nd_cur_name.len);

		// 处理特殊情况：'.'和'..'
		if(qstr_equals(nd->nd_cur_name, ".")) {
			;
		} else if(qstr_equals(nd->nd_cur_name, "..")) {
			error = follow_dotdot(nd);
		} else {
			error = do_lookup(nd);
		}

		if(IS_ERR_VALUE(error))
			break;
	}

	return error;
}

int do_lookup(nameidata *nd)
{
	int error = NO_ERROR;
	// 判断当前是否为目录
	if(!nd->nd_dentry->d_inode->i_isdir) {
		error = -ERROR_NOT_DIR;
		goto exit;
	}

	// 先在缓冲中找
	dentry *entry = lookup_cache(D_CACHE, nd);
	if(!entry) {
		// 没找到，需要在外存中找
		entry = real_lookup(nd);
		if(IS_ERR_PTR(entry)) {
			error = PTR_ERR(entry);
			goto exit;
		}
	}

	// 如果该目录项为挂载点，则跟踪挂载点切换文件系统
	vfsmount *mnt = nd->nd_mnt;
	while(entry->d_mounted) {
		list_head *head = get_sb_list_entry();
		list_head *p;
		list_for_each(p, head) {
			super_block *sb = container_of(p, super_block, s_listnode);
			if(sb->s_mnt.mnt_mntpoint == entry && sb->s_mnt.mnt_parent == mnt) {
				entry = sb->s_root;
				mnt = &sb->s_mnt;
				break;
			}
		}
		if(p == head) {
			error = -ERROR_VFSMOUNT;
			goto exit;
		}
	}

	// 更新当前分量的目录项和挂载信息
	d_dec_count(nd->nd_dentry);
	d_inc_count(entry);
	nd->nd_dentry = entry;
	nd->nd_mnt = mnt;

exit:
	return error;
}

dentry* real_lookup(const nameidata *nd)
{
	return nd->nd_dentry->d_inode->i_iop->lookup(nd->nd_dentry, nd->nd_cur_name);
}

int follow_dotdot(nameidata *nd)
{
	int error = NO_ERROR;
	dentry *entry = nd->nd_dentry;
	vfsmount *mnt = nd->nd_mnt;

	// 如果该目录项为当前文件系统的根目录，则跟踪挂载信息切换文件系统
	while(entry->d_sb->s_root == entry) {
		// 如果该目录项为整个操作系统的根目录，则直接返回
		if(entry == get_root_dentry())
			goto exit;
			
		entry = mnt->mnt_mntpoint;
		mnt = &entry->d_sb->s_mnt;
	}

	// 获得该目录项的父结点
	entry = get_dentry_parent(entry);
	if(IS_ERR_PTR(entry)) {
		error = PTR_ERR(entry);
		goto exit;
	}

	// 更新当前分量的目录项和挂载信息
	d_dec_count(nd->nd_dentry);
	d_inc_count(entry);
	nd->nd_dentry = entry;
	nd->nd_mnt = mnt;

exit:
	return error;
}

file* dentry_open_file(dentry* entry, uint mode)
{
	file *ret = (file*)kmalloc_fake(sizeof(file));
	if (!ret) {
		ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	d_inc_count(entry);
	ret->f_dirt 	= false;
	ret->f_dentry 	= entry;
	ret->f_mode 	= mode;
	ret->f_op 		= entry->d_inode->i_fop;
	ret->f_data_map = &entry->d_inode->i_data_map;

	ret->f_size 	= entry->d_inode->i_size;
	ret->f_pos 		= 0;
	ret->f_cur_off 	= 0;
	ret->f_cur_map 	= container_of(ret->f_data_map->next, fs_map, map_node);

	// 分析文件打开模式
	if(mode & F_MODE_APPEND_MASK) {
		ret->f_pos 		= ret->f_size;
		ret->f_cur_off 	= entry->d_sb->s_blocksize - file_last_blk_remain(ret);
		ret->f_cur_map	= container_of(ret->f_data_map->prev, fs_map, map_node);
	} else if(!(mode & F_MODE_READ_MASK)) {
		entry->d_inode->i_iop->clear(entry->d_inode, get_dentry_parent(entry), entry->d_name);
		ret->f_size 	= 0;
	}

	// kernel_printf("opened file: size = %d, pos = %d, cur_off = %d\n", ret->f_size, ret->f_pos, ret->f_cur_off);
exit:
	return ret;
}

dentry* get_dentry_parent(dentry* entry)
{
	dentry *ret;
	// 如果该目录项的父指针为空，说明父指针被清空，需要在外存找
	if(!entry->d_parent) {
		nameidata nd;
		nd.nd_cur_name.name 	= "..";
		nd.nd_cur_name.len 		= 2;
		nd.nd_dentry 			= entry;
		nd.nd_mnt 				= &entry->d_sb->s_mnt;

		ret = real_lookup(&nd);
	} else {
		ret = entry->d_parent;
	}
	return ret;
}

bool qstr_equals(const qstr qstr, const char* str)
{
	bool equals = true;
	const char *name = qstr.name;
	for(int i=0; i<qstr.len; i++) {
		if(!*str || *name != *str) {
			equals = false;
			break;
		}
		name ++;
		str ++;
	}

	return equals;
}

bool qstr_endwith(const char* filename, const qstr str)
{
	bool ret = true;

	int len = kernel_strlen(filename);
	if(len < str.len) {
		ret = false;
	} else {
		const char *name = str.name + str.len;
		const char *p = filename + len;

		while(len--) {
			if(*--p != *--name) {
				ret = false;
				break;
			}
		}
	}
	
	return ret;
}