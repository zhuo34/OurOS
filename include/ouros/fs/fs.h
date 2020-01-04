#ifndef _OUROS_FS_H_
#define _OUROS_FS_H_

#include <ouros/type.h>
#include <linux/list.h>

#define HOME_PATH 						"/home"

// 扇区字节数
#define SECTOR_SIZE 					512
#define SECTOR_SHIFT 					9

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
typedef struct _fs_map 					fs_map;
typedef struct _vfsmount 				vfsmount;
typedef struct _nameidata 				nameidata;
typedef struct _qstr 					qstr;
typedef struct list_head 				list_head;

#pragma pack(push)
#pragma pack(4)

// 文件系统类型
struct _file_system_type {
    const char 			*name; 						// 文件系统的名称
	// 函数接口，生成一个文件系统实例的超级块，输入参数为扇区地址
	super_block* 		(*get_sb) 		(DWord base_sec);
};

// 挂载信息
struct _vfsmount {
	vfsmount* 			mnt_parent;					// 父挂载信息
	dentry* 			mnt_mntpoint; 				// 挂载点目录项
	super_block* 		mnt_sb;						// 该文件系统的超级块
};

typedef struct _fs_info_fat32 			fs_info_fat32;
typedef struct _fs_info_ext2 			fs_info_ext2;

// 超级块，对应了一个文件系统实例的全局信息
struct _super_block {
	list_head 			s_listnode;					// 用于加入全局的超级块链表的结点
	file_system_type* 	s_type;						// 该文件系统的类型
	uint 				s_blocksize; 				// 数据块的字节大小
	uint 				s_blockbits; 				// 数据块的所占位数，用于移位
	uint 				s_basesec; 					// 该文件系统所有内容的起始扇区
	dentry* 			s_root; 					// 根结点的目录项
	super_block_op* 	s_op; 						// 超级块的操作函数
	vfsmount 			s_mnt; 						// 该文件系统的挂载信息
	
	union
	{
		fs_info_fat32* 	s_info_fat32;
		fs_info_ext2* 	s_info_ext2;
	}; 	// 对应操作系统的额外信息
};

// 超级块的操作
struct _super_block_op {
	// 根据信息构建索引结点
	inode* 		(*alloc_inode) 		(super_block* sb, uint base_blk_addr, uint size, bool isdir); 
	// 根据信息构建目录项
	dentry* 	(*alloc_dentry) 	(super_block* sb, inode* node, dentry* parent, const qstr name); 
	// 在外存上创建新的结点并构建相应的目录项
	dentry* 	(*create_dentry) 	(super_block* sb, dentry* parent, const qstr name, bool isdir);
};

// 块与缓存的映射信息
typedef struct _fspage 	fspage;
struct _fs_map
{
	uint 				map_addr; 					// 块的起始扇区地址
	fspage* 			map_page; 					// 对应的页缓存，如果为空则尚未写入缓存
	list_head 			map_node; 					// 加入inode中链表的结点
};

// 字符串
struct _qstr {
    uint 				len; 						// 字符串长度
    uint 				hash; 						// 哈希值
	const char* 		name; 						// 字符串内容
};

// 索引结点
struct _inode {
	uint 				i_ino; 						// 索引结点的序列号
	uint 				i_count;                	// 当前的引用计数
	uint 				i_size; 					// 文件字节大小
	uint 				i_blocks; 					// 文件所用的块数量
	bool 				i_pinned; 					// 是否被锁定
	bool 				i_isdir; 					// 判断该文件是否为目录
	list_head 			i_dentry; 					// 指向该索引结点的所有目录项的链表
	list_head			i_hash; 					// 用于加入哈希链表的结点
	list_head 			i_LRU; 						// 用于加入最近最少使用链表的结点
	list_head 			i_data_map; 				// 每一块的扇区地址与文件页缓存的map链表
	super_block* 		i_sb; 						// 该结点对应的超级块
	inode_op* 			i_iop; 						// 索引结点的操作
	file_op* 			i_fop; 						// 索引结点对应文件的操作
};

// 索引结点的操作
struct _inode_op {
	// 在外存中查找索引结点并返回对应的目录项
	dentry* 	(*lookup) 			(dentry* parent_entry, const qstr name);
	// 为inode申请新的block以扩展inode的大小，注意只申请了新的块，而没有改变inode的文件大小！
	int 		(*expand) 			(inode* node, int num);
	// 用于打开文件时设置打开模式为只写，将inode内容全部清空
	int 		(*clear) 			(inode* node, dentry* parent, const qstr name);
	// 在外存中删除inode
	int 		(*delete) 			(inode* node, dentry* parent, const qstr name);
};

// 目录项
struct _dentry {
	uint 				d_count;                	// 当前的引用计数
	bool 				d_pinned; 					// 是否被锁定
    bool 				d_mounted;              	// 该目录项上是否挂载了文件系统
	list_head			d_hash; 					// 用于加入哈希链表的结点
	list_head 			d_LRU; 						// 用于加入最近最少使用链表的结点
    list_head 			d_sibling;                	// 同一目录下目录项链表
    list_head 			d_subdirs;              	// 子目录项链表
    list_head 			d_alias;                	// 指向同一个索引结点的目录项链表
    super_block* 		d_sb; 						// 该目录项对应的超级块  
    inode* 				d_inode;               		// 对应的索引节点
    dentry* 			d_parent;              		// 父目录的目录项对象
    dentry_op* 			d_op; 						// 目录项的操作
    qstr 				d_name;                 	// 文件名
};

// 目录项的操作
struct _dentry_op {
	// 比较文件名，不同文件系统的比较方式不同，如FAT不区分大小写
	int 		(*compare)			(const qstr a, const qstr b);
	// 根据传入的目录项信息，打开一个文件
	file* 		(*open) 			(dentry* entry, uint mode);
};

// 打开文件的方式，即path_lookup函数参数flags
#define OPEN_FIND 			0x0000
#define OPEN_CREATE 		0x0001
#define OPEN_DIR 			0x0002

// 文件访问模式的掩码
#define F_MODE_READ_MASK 	0x0001
#define F_MODE_WRITE_MASK 	0x0002
#define F_MODE_APPEND_MASK 	0x0004
#define F_MODE_DIR_MASK 	0x0008
// 文件访问模式
#define F_MODE_READ 		F_MODE_READ_MASK 							// 只读模式
#define F_MODE_WRITE 		F_MODE_WRITE_MASK 							// 只写模式，打开一个已存在的文件会被清空
#define F_MODE_READ_WRITE 	(F_MODE_READ_MASK | F_MODE_WRITE_MASK) 		// 读写模式
#define F_MODE_APPEND 		(F_MODE_APPEND_MASK | F_MODE_READ_WRITE) 	// 追加模式

// fseek参数
enum {
	SEEK_CUR, SEEK_HEAD, SEEK_TAIL
};

// 文件
struct _file {
	uint 				f_size; 					// 文件的字节大小
	uint 				f_mode; 					// 读写文件的方式: read, write, append
    uint 				f_pos; 						// 文件当前的数据位置
	uint 				f_cur_off; 					// 文件当前的数据位置在一个缓存块中的偏移量
	fs_map* 			f_cur_map; 					// 文件当前数据位置对应的缓存块
	dentry* 			f_dentry; 					// 该文件对应的目录项
	file_op* 			f_op; 						// 文件的操作
	list_head* 			f_data_map; 				// 文件对应的数据映射链表
	bool 				f_dirt; 					// 文件是否被写过
};

// 文件操作
struct _file_op {
	// 如果该文件为目录，申请空间列出该目录下所有子文件；否则报错
	dentry*  	(*list_dir) 		(file* file);
	// 只修改父目录中的目录项信息，其他close操作在fs_close函数里完成
	int 		(*close) 			(file* file);
};

// 寻找目录的辅助结构
struct _nameidata {
	dentry* 			nd_dentry;					// 当前的目录项
	vfsmount* 			nd_mnt; 					// 当前的挂载信息
	qstr 				nd_cur_name; 				// 当前分量的名字
};

#pragma pack(pop)

// 函数定义
// fs.c
void init_fs();
list_head* get_sb_list_entry();
dentry* get_root_dentry();
dentry* get_pwd_dentry();
void set_pwd_dentry(dentry* entry);
DWord read_data(void *addr, uint size);

void d_inc_count(dentry* entry);
void d_dec_count(dentry* entry);
void i_inc_count(inode*  entry);
void i_dec_count(inode*  entry);

// open.c
file* fs_open(const char *filename, uint mode);
int fs_close(file *fp);

file* dentry_open_file(dentry* entry, uint mode);
dentry* get_dentry_parent(dentry* entry);

// read_write.c
int fs_read(file *fp, void *buf, uint len);
char fs_getchar(file *fp);

int fs_write(file *fp, void *buf, uint len);
int fs_printf(file *fp, const char *format, ...);
int fs_puts(file *fp, const char *str);
int fs_putchar(file *fp, char c);
int fs_putint(file *fp, int num);

int fs_seek(file *fp, int flag, int offset);

int fs_delete(file *fp);

// inline functions
static inline bool eof(file *fp) 
{ 
	return fp->f_pos == fp->f_size; 
}

static inline int file_last_blk_remain(file *fp)
{
	inode *node = fp->f_dentry->d_inode;
	super_block *sb = node->i_sb;
	return node->i_blocks * sb->s_blocksize - fp->f_size;
}

extern void* kmalloc (uint size);
extern void  kfree (void* p);

#endif