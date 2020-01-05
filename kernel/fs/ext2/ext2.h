#ifndef EXT2_H
#define EXT2_H

#include <ouros/fs/ext2.h>

#define 	MAX_NAME_LEN_EXT2 					255
#define 	MAX_BLOCKS_EXT2 					15
#define 	BLOCK_SIZE_EXT2 					1024

// ext2文件系统的超级块
typedef struct _super_block_ext2 {
	DWord 			sb_inode_num; 					// 文件系统的索引结点数
	DWord 			sb_blk_num; 					// 文件系统的块数
	DWord 			sb_reserved_blk_num; 			// 文件系统的保留块数
	DWord 			sb_free_blk_num; 				// 文件系统的空闲块数
	DWord 			sb_free_inode_num; 				// 文件系统的空闲inode数
	DWord 			sb_1st_datablk_no; 				// 第一个数据块号
	DWord 			sb_blk_shift; 					// 块移位数，以1k为基准(1k << shift)
	DWord 			sb_slice_shift; 				// 片移位数，以1k为基准(1k << shift)
	DWord 			sb_blk_pgroup; 					// 每组的块数
	DWord 			sb_slice_pgroup; 				// 每组的片数
	DWord 			sb_inode_pgroup; 				// 每组的索引结点数
	DWord 			sb_install_time; 				// 文件系统安装时间
	DWord 			sb_write_time; 					// 最后写入的时间
	Word 			sb_install_cnt; 				// 安装计数
	Word 			sb_install_cnt_max; 			// 最大安装数
	Word 			sb_magic_num; 					// 魔术数字
	Word 			sb_state; 						// 文件系统状态
	Word 			sb_err_act; 					// 出错动作
	Word 			sb_version_change; 				// 改版标志
	DWord 			sb_last_check; 					// 最后检测时间
	DWord 			sb_max_check_interval; 			// 最大检测间隔
	DWord 			sb_os; 							// 操作系统
	DWord 			sb_version; 					// 版本标志
	Word 			sb_uid; 						// 用户id
	Word 			sb_gid; 						// 组id
	DWord 			sb_1st_inode; 					// 第一个非保留的inode
	Word 			sb_inode_size; 					// inode的大小
}super_block_ext2;


#define EXT2_FILE_TYPE_UNKNOWN 		0
#define EXT2_FILE_TYPE_NORMAL 		1
#define EXT2_FILE_TYPE_DIR 			2

// ext2文件系统的目录项
typedef struct _dentry_ext2 {
	DWord 			d_ino; 							// 文件的ino号
	Word 			d_size; 						// 目录项字节大小
	Byte 			d_name_len; 					// 文件名字节长度
	Byte 			d_type; 						// 文件类型
	Byte 			d_name[MAX_NAME_LEN_EXT2]; 		// 文件名
}dentry_ext2;

// ext2文件系统的索引结点
typedef struct _inode_ext2 {
	Word 			i_mode; 						// 文件打开模式
	Word 			i_uid; 							// uid的低16位
	DWord 			i_size; 						// 文件字节大小
	DWord 			i_atime; 						// 最近访问时间
	DWord 			i_ctime; 						// 创建时间
	DWord 			i_mtime; 						// 修改时间
	DWord 			i_dtime; 						// 删除时间
	Word 			i_gid; 							// gid的低16位
	Word 			i_links; 						// 链接数量
	DWord 			i_blocks; 						// 文件块数
	DWord 			i_flags; 						// 文件打开标记
	DWord 			i_reserved1; 					// 保留字段1
	DWord 			i_block_addr[MAX_BLOCKS_EXT2]; 	// 存放所有相关的块地址
	DWord 			i_version; 						// 文件版本
	DWord 			i_file_acl; 					// 文件的ACL
	DWord 			i_dir_acl; 						// 目录的ACL
	DWord 			i_frag_addr; 					// 碎片地址
	DWord 			i_reserved2[3]; 				// 保留字段2
}inode_ext2;

// ext2文件系统的组描述符
typedef struct _group_ext2 {
	DWord 			g_blk_bmp; 						// 块位图所在块
	DWord 			g_inode_bmp; 					// inode位图所在块
	DWord 			g_inode_table; 					// inode列表所在块
	Word 			g_free_blocks; 					// 空闲块数
	Word 			g_free_inodes; 					// 空闲节点数
	Word 			g_used_dirs; 					// 目录数
	Word 			g_reserved1; 					// 保留字段1
	DWord 			g_reserved2[3]; 				// 保留字段2
}group_ext2;

// ext2文件系统信息
struct _fs_info_ext2 {
	DWord 			info_1st_sb_sec; 				// 第一个超级块的起始地址
	DWord 			info_1st_gdt_sec; 				// 第一个组描述符表的起始地址 
	DWord 			info_inode_pgroup; 				// 每个组中的inode数
	DWord 			info_inode_size; 				// inode的大小
};

// fs_type op
super_block* get_sb_ext2(DWord base_sec);

// super_block_op
inode* 	alloc_inode_ext2(super_block* sb, uint base_blk_addr, uint size, bool isdir);
dentry* alloc_dentry_ext2(super_block* sb, inode* node, dentry* parent, const qstr name);
dentry* create_dentry_ext2(super_block* sb, dentry* parent, const qstr name, bool isdir);

// inode_op
dentry* lookup_ext2(dentry* parent_entry, const qstr name);
int expand_ext2(inode* node, int num);
int clear_ext2(inode* node, dentry* parent, const qstr name);
int delete_ext2(inode* node, dentry* parent, const qstr name);

// dentry_op
int compare_ext2(const qstr a, const qstr b);

// file op
dentry* list_dir_ext2(file* file);
int close_ext2(file* file);

uint find_page_sec_ext2(inode *node, int curPage, uint* pageaddr);

#endif  // EXT2_H
