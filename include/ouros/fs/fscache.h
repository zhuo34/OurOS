#ifndef _OUROS_FS_FSCACHE_H_
#define _OUROS_FS_FSCACHE_H_

#include <ouros/fs/fs.h>

// 初始化文件系统
int init_fscache();

// 目录项缓存操作
dentry* lookup_dcache(const nameidata *nd);

// 索引结点缓存操作
inode* lookup_icache(const nameidata *nd);

#endif