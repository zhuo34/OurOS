#ifndef _OUROS_FS_H_
#define _OUROS_FS_H_

#include <ouros/type.h>
#include <ouros/list.h>

// 扇区字节数
#define SECTOR_SIZE 					512
#define SECTOR_SHIFT 					9

// 打开文件的方式，即open函数参数flags
#define OPEN_FIND 						0x0000
#define OPEN_CREATE 					0x0001

// 文件访问模式
#define FMODE_READ 						0x0000 				// 只读模式
#define FMODE_WRITE 					0x0001 				// 读写模式
#define FMODE_APPEND 					0x0002 				// 追加模式

// 结构体类型定义
typedef struct _file_system_type 		file_system_type;
typedef struct _super_block 			super_block;
typedef struct _super_block_op 			super_block_op;
typedef struct _inode 					inode;
typedef struct _inode_op				inode_op;
typedef struct _dentry 					dentry;
typedef struct _dentry_op				dentry_op;
typedef struct _file 					file;
typedef struct _file_op					file_op;
typedef struct _address_space 			address_space;
typedef struct _vfsmount 				vfsmount;
typedef struct _nameidata 				nameidata;
typedef struct _qstr 					qstr;
typedef struct list_head 				list_head;

// 文件系统类型
struct _file_system_type {
    const char 			*name; 						// 文件系统的名称
	// 函数接口，生成一个文件系统实例的超级块
	super_block* 		(*get_sb) 		(DWord base);
};

// 挂载信息
struct _vfsmount {
	vfsmount* 			mnt_parent;					// 父挂载信息
	dentry* 			mnt_mntpoint; 				// 挂载点目录项
	super_block* 		mnt_sb;						// 该文件系统的超级块
};

// 超级块，对应了一个文件系统实例的全局信息
struct _super_block {
	list_head 			s_listnode;					// 用于加入全局的超级块链表的结点
	file_system_type* 	s_type;						// 该文件系统的类型
	uint 				s_blocksize; 				// 数据块的字节大小
	uint 				s_blockbits; 				// 数据块的所占位数，用于移位
	dentry* 			s_root; 					// 根结点的目录项
	super_block_op* 	s_op; 						// 超级块的操作函数
	vfsmount 			s_mnt; 						// 该文件系统的挂载信息
};

// 超级块的操作
struct _super_block_op {
	inode* 		(*create_inode) 	(); 			// 创建索引结点
	bool 		(*delete_inode) 	(inode*);		// 删除索引结点
	bool 		(*read_inode) 		(inode*); 		// 在外存上读取该索引结点的数据
	bool 		(*write_inode) 		(inode*); 		// 向外存中写入该索引结点的数据
};

// 页缓存
struct _address_space {
	inode* 				a_host; 					// 对应的索引结点
	list_head 			a_
};

// 索引结点
struct _inode {
	bool 				i_pinned; 					// 是否被锁定
	uint 				i_ino; 						// 索引结点的序列号
	uint 				i_size; 					// 文件字节大小
	uint 				i_blocks; 					// 文件所用的块数量
	bool 				i_isdir; 					// 判断该文件是否为目录
	super_block* 		i_sb; 						// 该结点对应的超级块
	list_head 			i_dentry; 					// 指向该索引结点的所有目录项的链表
	list_head			i_hash; 					// 用于加入哈希链表的结点
	list_head 			i_LRU; 						// 用于加入最近最少使用链表的结点
	inode_op* 			i_iop; 						// 索引结点的操作
	file_op* 			i_fop; 						// 索引结点对应文件的操作
	address_space 		i_data; 					// 该结点的实际数据
};

// 索引结点的操作
struct _inode_op {
	int 		(*create) 			(inode *, dentry *, int);
	dentry* 	(*lookup) 			(inode *, dentry *);
	int 		(*link) 			(dentry *, inode *, dentry *);
	int 		(*unlink) 			(inode *, dentry *);
	int 		(*symlink) 			(inode *, dentry *, const char *);
	int 		(*mkdir) 			(inode *, dentry *, int);
	int 		(*rmdir) 			(inode *, dentry *);
	int 		(*rename) 			(inode *, dentry *, inode *, dentry *);
	int 		(*setattr) 			(dentry *);
	int 		(*getattr) 			(vfsmount *, dentry *);
};

// 目录项
struct _dentry {
	bool 				d_pinned; 					// 是否被锁定
	list_head			d_hash; 					// 用于加入哈希链表的结点
	list_head 			d_LRU; 						// 用于加入最近最少使用链表的结点
	uint 				d_count;                	// 当前的引用计数
    bool 				d_mounted;              	// 该目录项上是否挂载了文件系统
    inode* 				d_inode;               		// 对应的索引节点
    dentry* 			d_parent;              		// 父目录的目录项对象
    qstr 				d_name;                 	// 文件名
    list_head 			d_sibling;                	// 同一目录下目录项链表
    list_head 			d_subdirs;              	// 子目录项链表
    list_head 			d_alias;                	// 指向同一个索引结点的目录项链表
    dentry_op* 			d_op; 						// 目录项的操作
    super_block* 		d_sb; 						// 该目录项对应的超级块  
};

// 目录项的操作
struct _dentry_op {
	// 比较文件名，不同文件系统的比较方式不同，如FAT不区分大小写
	int 		(*compare)			(const qstr a, const qstr b);
};

// 文件
struct _file {
    uint 				f_pos;                  	// 文件当前的数据位置
	dentry* 			f_dentry;              		// 该文件对应的目录项
	vfsmount* 			f_mnt;              		// 该文件对应的挂载信息
	file_op* 			f_op;                  		// 文件的操作
	uint 				f_flags;                	// 打开的文件方式
	uint 				f_mode;                 	// 进程访问文件的方式
	address_space* 		f_mapping;             		// 文件地址空间
};

// 文件操作
struct _file_op {

};

// 字符串
struct _qstr {
    const char* 		name; 						// 字符串内容
    uint 				len; 						// 字符串长度
    uint 				hash; 						// 哈希值
};

// 寻找目录的辅助结构
struct _nameidata {
	dentry* 			nd_dentry;					// 当前的目录项
	vfsmount* 			nd_mnt; 					// 当前的挂载信息
	qstr 				nd_cur_name; 				// 当前分量的名字
};

// 函数定义
// fs.c
void init_fs();
dentry* get_root_dentry();
dentry* get_pwd_dentry();
DWord read_data(void *addr, uint size);

// open.c
file* fs_open(const char *filename, int flags, int mode);
int fs_close(file *fp);

// read_write.c
int fs_read(file *fp, Byte *buf, uint len);
int fs_write(file *fp, Byte *buf, uint len);

// 假的malloc操作
void* kmalloc_fake(uint size);
void  kfree_fake(void* p);

#endif