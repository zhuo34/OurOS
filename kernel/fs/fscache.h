#ifndef _FS_FSCACHE_H_
#define _FS_FSCACHE_H_

#include <ouros/fs/fscache.h>

#define HASH_SIZE 			64
#define HASH_MASK 			(HASH_SIZE - 1)

#define DCACHE_CAPACITY 	64
#define ICACHE_CAPACITY 	64
#define PCACHE_CAPACITY 	128

// 初始化
void init_fscache_info(fscache *cache, int capacity);

// 加入缓存
void add_to_d_cache(fscache* cache, void *item);
void add_to_i_cache(fscache* cache, void *item);
void add_to_p_cache(fscache* cache, void *item);

// 将满缓存的一部分写回硬盘
bool flush_cache(int cache_type);
bool flush_d_cache(fscache* cache);
bool flush_i_cache(fscache* cache);
bool flush_p_cache(fscache* cache);
void free_dentry(dentry* entry);
void free_fspage(fspage* page);

// 从缓存中移除
bool del_from_d_cache(fscache* cache, void *item);
bool del_from_i_cache(fscache* cache, void *item);
bool del_from_p_cache(fscache* cache, void *item);

// 在缓存中查找
dentry* lookup_d_cache(fscache* cache, const nameidata *nd);
inode* 	lookup_i_cache(fscache* cache, uint base_sec);
fspage* lookup_p_cache(fscache* cache);

// 哈希操作
uint hash_string(qstr key);
uint hash_int(uint key);

#endif