#ifndef EXT2_H
#define EXT2_H

#include <ouros/fs/ext2.h>

// // fs_type op
// super_block* get_sb_ext2(DWord base_sec);

// // super_block_op
// inode* 	alloc_inode_ext2(super_block* sb, uint base_blk_addr, uint size, bool isdir);
// dentry* alloc_dentry_ext2(super_block* sb, inode* node, dentry* parent, const qstr name);
// dentry* create_dentry_ext2(super_block* sb, dentry* parent, const qstr name, bool isdir);

// // inode_op
// dentry* lookup_ext2(dentry* parent_entry, const qstr name);
// int expand_ext2(inode* node, int num);
// int clear_ext2(inode* node, dentry* parent, const qstr name);
// int delete_ext2(inode* node, dentry* parent, const qstr name);

// dentry_op
int compare_ext2(const qstr a, const qstr b);

// // file op
// dentry* list_dir_ext2(file* file);
// int close_ext2(file* file);

#endif  // EXT2_H
