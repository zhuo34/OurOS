#ifndef _OUROS_FS_FSCACHE_H_
#define _OUROS_FS_FSCACHE_H_

#include <ouros/fs/fs.h>

// 缓存类型
enum {
	D_CACHE, I_CACHE, P_CACHE
};
#define CACHE_TYPE_CNT 	3

#pragma pack(push)
#pragma pack(4)

// 文件页缓存
struct _fspage
{
	bool 			p_dirt; 		// 判断是否为脏页
	Byte* 			p_data; 		// 缓存的数据
	uint 			p_pagesize; 	// 缓存数据的字节大小
	uint 			p_sec_addr; 	// 对应数据的扇区起始地址
	uint 			p_sec_cnt; 		// 对应数据的扇区个数
	list_head		p_hash; 		// 用于加入哈希链表的结点
	list_head 		p_LRU; 			// 用于加入最近最少使用链表的结点
};

typedef struct _fscache
{
	uint 			size; 			// 当前已缓存的个数
	uint 			capacity; 		// 最大可缓存个数
	list_head 		LRU; 			// 最近最少使用
	list_head* 		hash; 			// 哈希表
}fscache;

#pragma pack(pop)

// 文件页操作
fspage* alloc_fspage(uint base_sec, uint blk_size);
int flush_fspage(fspage* page);
fspage* get_fspage(list_head* fs_map_entry, uint blksize);

// 缓存管理操作
bool add_to_cache(int cache_type, void *item);
void* lookup_cache(int cache_type, ...);
fscache* get_fscache(int cache_type);

// 释放缓存操作
void free_inode (inode*  node);

// 初始化文件系统缓存
int init_fscache();

#endif