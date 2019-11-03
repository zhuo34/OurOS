#ifndef _OS_FS_H_
#define _OS_FS_H_

#include <ouros/type.h>

typedef struct _list_head 			List_Head;
typedef struct _super_block 		Super_Block;
typedef struct _super_block_op 		Super_Block_OP;
typedef struct _file_system_type 	FS_Type;
typedef struct _inode 				INode;
typedef struct _inode_op 			Inode_OP;
typedef struct _string 				String;
typedef struct _dentry 				DEntry;
typedef struct _dentry_op 			DEntry_OP;
typedef struct _file 				File;
typedef struct _file_op 			File_OP;

struct _list_head {
	struct _list_head *prev;
	struct _list_head *next;
};

struct _super_block {
    uint 					s_blocksize; 			// 块的字节大小
    bool 					s_dirt; 				// 脏块标记
    Super_Block_OP 			*s_op; 					// 超级块的方法接口
    DEntry            		*s_root; 				// 文件系统的根结点目录项
};

struct _super_block_op {
	// 创建inode对象
	INode* 	(*create_inode) 	();
	// 删除inode对象
	void 	(*delete_inode) 	(INode *);
	// 从外存中读inode的数据
	void 	(*read_inode) 		(INode *);
	// 向外存中写inode的数据
	void 	(*write_inode) 		(INode *, int);
};

struct _file_system_type {
    const char 				*name; 					// 文件系统的名称
	Super_Block 			(*get_super_block)();	// 函数接口，获得该文件系统对应的超级块
};

// struct _inode {
// 	List_Head 				i_hash; 				// 指向哈希表的链表指针
// 	uint 					i_ino; 					// 索引结点号
// 	uint 					i_count; 				// 引用计数
// 	uint 					i_size; 				// 文件的字节大小
// 	uint 					i_blksize; 				// 块的字节大小
// 	uint 					i_blocks; 				// 文件占用的块数
// 	Inode_OP 				*i_op; 					// 索引结点的方法接口
// 	Super_Block 			*i_sb; 					// 对应的超级块
// 	// struct address_space    *i_mapping;          /* associated mapping */
// 	// struct address_space    i_data;              /* mapping for device */
// };

// struct _inode_op {
// 	int 	(*create) 			(INode *, DEntry *,int);
// 	DEntry* (*lookup) 			(INode *, DEntry *);
// 	int 	(*link) 			(DEntry *, INode *, DEntry *);
// 	int 	(*unlink) 			(INode *, DEntry *);
// 	int 	(*symlink) 			(INode *, DEntry *, const char *);
// 	int 	(*mkdir) 			(INode *, DEntry *, int);
// 	int 	(*rmdir) 			(INode *, DEntry *);
// 	int 	(*rename) 			(INode *, DEntry *, INode *, DEntry *);
// 	int 	(*readlink) 		(DEntry *, char *, int);
// 	int 	(*follow_link) 		(DEntry *, struct nameidata *);
// 	int 	(*put_link) 		(DEntry *, struct nameidata *);
// };

// struct _string {
// 	const char 				*name; 					// 字符串
// 	uint 					len;					// 长度
// 	uint 					hash; 					// 哈希值
// };

// struct _dentry {
// 	uint 					d_count; 				// 引用计数
// 	INode 					*d_inode; 				// 指向的inode对象
// 	List_Head 				d_LRU; 					// 用于LRU链表的指针 -- 最近最少使用
// 	List_Head 				d_child; 				// 子目录链表指针
// 	List_Head 				d_siblings; 			// 当前目录链表指针
// 	List_Head 				d_alias; 				// 指向同一个inode的目录项链表
// 	DEntry_OP 				*d_op; 					// 目录项函数接口
// 	Super_Block 			*d_sb; 					// 对应的超级块
// 	uint 					d_mounted; 				// 该目录下挂载的文件系统数量
// 	DEntry 					*d_parent; 				// 指向父结点的指针
// 	String 					d_name; 				// 目录项的名称
// 	List_Head 				d_hash; 				// 用于哈希表的指针
// };

// struct _dentry_op {
// 	int 	(*d_revalidate) 	(struct dentry *, int);
// 	int 	(*d_hash) 			(struct dentry *, struct qstr *);
// 	int 	(*d_compare) 		(struct dentry *, struct qstr *, struct qstr *);
// 	int 	(*d_delete) 		(struct dentry *);
// 	void 	(*d_release) 		(struct dentry *);
// 	void 	(*d_iput) 			(struct dentry *, struct inode *);
// };

// struct _file {
// 	List_Head 				f_list;        /* list of file objects */
// 	DEntry 					*f_dentry;     /* associated dentry object */
// 	// struct vfsmount        *f_vfsmnt;     /* associated mounted fs */
// 	File_OP 				*f_op;         /* file operations table */
// 	uint 					f_flags;       /* flags specified on open */
// 	uint 					f_mode;        /* file access mode */
// 	uint 					f_pos;         /* file offset (file pointer) */
// 	// struct address_space   *f_mapping;    /* page cache mapping */
// };

// struct _file_op {
// 	uint (*read) (File *, char *, uint, uint *);
// 	uint (*write) (File *, const char *, uint, uint *);
// 	uint (*readdir) (File *, void *, uint);
// 	uint (*open) (INode *, File *);
// 	uint (*flush) (File *);
// 	uint (*check_flags) (int flags);
// };

// struct vfsmount {
// 	struct list_head   mnt_hash;        /* hash table list */
// 	struct vfsmount    *mnt_parent;     /* parent filesystem */
// 	struct dentry      *mnt_mountpoint; /* dentry of this mount point */
// 	struct dentry      *mnt_root;       /* dentry of root of this fs */
// 	struct super_block *mnt_sb;         /* superblock of this filesystem */
// 	struct list_head   mnt_mounts;      /* list of children */
// 	struct list_head   mnt_child;       /* list of children */
// 	atomic_t           mnt_count;       /* usage count */
// 	int                mnt_flags;       /* mount flags */
// 	struct namespace   *mnt_namespace;   /* associated namespace */
// };

void init_fs();

#endif