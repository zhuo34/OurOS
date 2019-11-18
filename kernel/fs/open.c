#include "open.h"

#include <ouros/error.h>
#include <ouros/utils.h>
#include <ouros/fs/fscache.h>

file* fs_open(const char *filename, int flags, int mode)
{
	nameidata nd;
	int error = path_lookup(filename, flags, &nd);

	return IS_ERR_VALUE(error)?
				ERR_PTR(error):
				dentry_open(nd.nd_dentry, nd.nd_mnt, mode);
}

int fs_close(file *fp)
{

}

int path_lookup(const char *filename, int flags, nameidata *nd)
{
	dentry* entry = (*filename == '/')? get_root_dentry(): get_pwd_dentry();
	dget(entry);
	nd->nd_dentry = entry;
	nd->nd_mnt = &entry->d_sb->s_mnt;

	int error = link_path_walk(filename, nd);
	if(error == -ERROR_FILE_NOTFOUND && flags == OPEN_CREATE
			&& qstr_endwith(filename, nd->nd_cur_name)) {
		entry = nd->nd_dentry->d_sb->s_op->create(nd->nd_dentry, nd->nd_cur_name);
		error = IS_ERR_PTR(entry)? PTR_ERR(entry): NO_ERROR;
	}
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

bool qstr_equals(const qstr qstr, const char* str)
{
	bool equals = true;
	char *name = qstr.name;
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
	int len = 0;
	char *p = filename;
	while(*p++)
		len ++;
	
	bool ret = true;
	if(len < str.len)
		ret = false;
	else {
		char *name = str.name + str.len;
		p = filename + len;
		while(len--) {
			if(*--p != *--name) {
				ret = false;
				break;
			}
		}
	}
	
	return ret;
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
	dentry *entry = lookup_dcache(nd);
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
		list_node *head = &entry->d_sb->s_listnode;
		list_node *p;
		for(p = head->next; head != p; p = p->next) {
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
	dput(nd->nd_dentry);
	dget(entry);
	nd->nd_dentry = entry;
	nd->nd_mnt = mnt;

exit:
	return error;
}

dentry* real_lookup(const nameidata *nd)
{
	super_block *sb = nd->nd_dentry->d_sb;
	dentry *entry = sb->s_op->lookup(nd->nd_dentry, nd->nd_cur_name);
	return entry;
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

	// 如果该目录项的父指针为空，说明父指针被清空，需要在外存找
	if(!entry->d_parent) {
		entry = real_lookup(nd);
		if(IS_ERR_PTR(entry)) {
			error = PTR_ERR(entry);
			goto exit;
		}
	} else {
		entry = entry->d_parent;
	}

	// 更新当前分量的目录项和挂载信息
	dput(nd->nd_dentry);
	dget(entry);
	nd->nd_dentry = entry;
	nd->nd_mnt = mnt;

exit:
	return error;
}