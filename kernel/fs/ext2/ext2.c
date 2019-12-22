#include "ext2.h"

#include <driver/sd.h>
#include <driver/vga.h>

#include <ouros/error.h>
#include <ouros/fs/fscache.h>

static file_system_type fs_type_ext2 = {
	.name 				= 		"ext2",
	// .get_sb 			= 		get_sb_ext2,
};

static super_block_op sb_op_ext2 = {
// 	.alloc_inode 		=		alloc_inode_ext2,
// 	.alloc_dentry 		=		alloc_dentry_ext2,
// 	.create_dentry 		= 		create_dentry_ext2,
};

static inode_op inode_op_ext2 = {
// 	.lookup 			= 		lookup_ext2,
// 	.expand 			= 		expand_ext2,
// 	.clear 				= 		clear_ext2,
// 	.delete 			= 		delete_ext2,
};

static dentry_op dentry_op_ext2 = {
	.compare 			= 		compare_ext2,
	.open 				= 		dentry_open_file,
};

static file_op file_op_ext2 = {
// 	.list_dir 			= 		list_dir_ext2,
// 	.close 				= 		close_ext2,
};

file_system_type* get_fs_type_ext2()
{
	return &fs_type_ext2;
}

// super_block* get_sb_ext2(DWord base_sec);

// inode* 	alloc_inode_ext2(super_block* sb, uint base_blk_addr, uint size, bool isdir);
// dentry* alloc_dentry_ext2(super_block* sb, inode* node, dentry* parent, const qstr name);
// dentry* create_dentry_ext2(super_block* sb, dentry* parent, const qstr name, bool isdir);

// dentry* lookup_ext2(dentry* parent_entry, const qstr name);
// int expand_ext2(inode* node, int num);
// int clear_ext2(inode* node, dentry* parent, const qstr name);
// int delete_ext2(inode* node, dentry* parent, const qstr name);

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

// dentry* list_dir_ext2(file* file);
// int close_ext2(file* file);