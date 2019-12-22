#include "fscache.h"

#include <ouros/error.h>

#include <driver/vga.h>
#include <driver/sd.h>

static fscache *cache_array;

fscache* get_fscache(int cache_type)
{
	return &cache_array[cache_type];
}

fspage* alloc_fspage(uint base_sec, uint blk_size)
{
	fspage *page = (fspage*)kmalloc_fake(sizeof(fspage));
	if(!page) {
		page = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	page->p_data = (Byte*)kmalloc_fake(blk_size);
	if(!page->p_data) {
		kfree_fake(page);
		page = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	page->p_dirt 		= false;
	page->p_pagesize 	= blk_size;
	page->p_sec_cnt 	= blk_size >> SECTOR_SHIFT;
	page->p_sec_addr 	= base_sec;
	INIT_LIST_HEAD(&page->p_LRU);
	INIT_LIST_HEAD(&page->p_hash);

	// 读数据
	if(!sd_read_block(page->p_data, base_sec, page->p_sec_cnt)) {
		kfree_fake(page->p_data);
		kfree_fake(page);
		page = ERR_PTR(-ERROR_READ_SDCARD);
	}

	if(!add_to_cache(P_CACHE, page))
		page = ERR_PTR(-ERROR_CACHE_FULL);
	
exit:
	return page;
}

int flush_fspage(fspage* page)
{
	int error = NO_ERROR;
	if(!page || !page->p_dirt) {
		;
	} else if(!sd_write_block(page->p_data, page->p_sec_addr, page->p_sec_cnt)) {
		error = -ERROR_READ_SDCARD;
	} else {
		page->p_dirt = false;
	}
	return error;
}

fspage* get_fspage(list_head* fs_map_entry, uint blksize)
{
	fs_map* map = container_of(fs_map_entry, fs_map, map_node);
	fspage* cur_page = map->map_page;
	if(!cur_page) {
		cur_page = alloc_fspage(map->map_addr, blksize);
		map->map_page = cur_page;
	}
	return cur_page;
}

// 初始化
int init_fscache()
{
	int error = NO_ERROR;
	cache_array = (fscache*)kmalloc_fake(sizeof(fscache) * CACHE_TYPE_CNT);
	if(!cache_array) {
		error = -ERROR_NO_MEMORY;
	} else {
		init_fscache_info(&cache_array[D_CACHE], DCACHE_CAPACITY);
		init_fscache_info(&cache_array[I_CACHE], ICACHE_CAPACITY);
		init_fscache_info(&cache_array[P_CACHE], PCACHE_CAPACITY);
	}

	return error;
}

void init_fscache_info(fscache *cache, int capacity)
{
	cache->size = 0;
	cache->capacity = capacity;
	cache->hash = (list_head*)kmalloc_fake(sizeof(list_head) * HASH_SIZE);
	for(int i=0; i<HASH_SIZE; i++) {
		INIT_LIST_HEAD(&cache->hash[i]);
	}
	INIT_LIST_HEAD(&cache->LRU);
}

// 加入缓存
bool add_to_cache(int cache_type, void *item)
{
	bool ret = true;
	fscache *cache = &cache_array[cache_type];
	// kernel_printf("add to cache[%d], size = %d, addr = %X\n", cache_type, cache->size, item);
	if(cache->size == cache->capacity && !flush_cache(cache_type)) {
		ret = false;
	} else if(cache_type == D_CACHE) {
		add_to_d_cache(cache, item);
	} else if(cache_type == I_CACHE) {
		add_to_i_cache(cache, item);
	} else if(cache_type == P_CACHE) {
		add_to_p_cache(cache, item);
	}
	return ret;
}

void add_to_d_cache(fscache* cache, void *item)
{
	dentry* entry = (dentry*)item;
	
	int hash = hash_string(entry->d_name);
	list_add(&entry->d_hash, &cache->hash[hash]);
	list_add(&entry->d_LRU, &cache->LRU);

	cache->size ++;
}

void add_to_i_cache(fscache* cache, void *item)
{
	inode* node = (inode*)item;
	
	int hash = hash_int(node->i_ino);
	list_add(&node->i_hash, &cache->hash[hash]);
	list_add(&node->i_LRU, &cache->LRU);

	cache->size ++;
}

void add_to_p_cache(fscache* cache, void *item)
{
	fspage* page = (fspage*)item;
	
	// list_add(&page->p_hash, &cache->hash);
	list_add(&page->p_LRU, &cache->LRU);

	cache->size ++;
}

// 将满缓存的一部分写回硬盘
bool flush_cache(int cache_type)
{
	bool ret = false;
	fscache *cache = &cache_array[cache_type];
	if(cache_type == D_CACHE) {
		ret = flush_d_cache(cache);
	} else if(cache_type == I_CACHE) {
		ret = flush_i_cache(cache);
	} else if(cache_type == P_CACHE) {
		ret = flush_p_cache(cache);
	}
	// kernel_printf("flush cache[%d], size = %d\n", cache_type, cache->size);
	return ret;
}

bool flush_d_cache(fscache* cache)
{
	bool ret = false;
	
	dentry *target = nullptr;
	list_head *p;
	list_for_each_prev(p, &cache->LRU) {
		dentry *entry = container_of(p, dentry, d_LRU);
		if(entry->d_pinned) {
			;
		} else if(entry->d_count == 0) {
			p = entry->d_LRU.next;

			ret = true;
			free_dentry(entry);
			cache->size --;
		} else if(!target) {
			target = entry;
		}
	}
	if(!ret && target) {
		ret = true;
		free_dentry(target);
		cache->size --;
	}

	return ret;
}

bool flush_i_cache(fscache* cache)
{
	bool ret = false;

	inode *target = nullptr;
	list_head *p;
	list_for_each_prev(p, &cache->LRU) {
		inode *node = container_of(p, inode, i_LRU);
		if(node->i_pinned) {
			;
		} else if(node->i_count == 0) {
			p = node->i_LRU.next;

			ret = true;
			free_inode(node);
			cache->size --;
		} else if(!target) {
			target = node;
		}
	}
	if(!ret && target) {
		ret = true;
		free_inode(target);
		cache->size --;
	}

	return ret;
}

bool flush_p_cache(fscache* cache)
{
	bool ret = true;
	list_head *target = &cache->LRU;
	for(int i = 0; i < PCACHE_CAPACITY / 2; i++) {
		target = target->prev;
		fspage *page = container_of(target, fspage, p_LRU);
		if(IS_ERR_VALUE(flush_fspage(page))) {
			ret = false;
			goto exit;
		}
		free_fspage(page);
		cache->size --;
	}

exit:
	return ret;
}

void free_dentry(dentry* entry)
{
	list_del(&entry->d_LRU);
	list_del(&entry->d_hash);
	list_del(&entry->d_alias);
	list_del(&entry->d_sibling);
	
	list_head *p;
	list_for_each(p, &entry->d_subdirs) {
		container_of(p, dentry, d_sibling)->d_parent = nullptr;
	}

	if((DWord)entry->d_name.name > 0x81000000) {
		kfree_fake((void*)entry->d_name.name);
	}
	kfree_fake(entry);
}

void free_inode (inode*  node)
{
	list_del(&node->i_LRU);
	list_del(&node->i_hash);
	
	list_head *p;
	list_for_each(p, &node->i_dentry) {
		free_dentry(container_of(p, dentry, d_alias));
	}
	list_for_each(p, &node->i_data_map) {
		list_head *cur = p;
		p = p->prev;
		list_del(cur);
		fs_map *map = container_of(cur, fs_map, map_node);
		free_fspage(map->map_page);
		kfree_fake(map);
	}
	
	kfree_fake(node);
}

void free_fspage(fspage* page)
{
	if(!page)
		return;

	list_del(&page->p_LRU);
	list_del(&page->p_hash);

	container_of(page, fs_map, map_page)->map_page = nullptr;

	kfree_fake(page->p_data);
	kfree_fake(page);
}

// 在缓存中查找
void* lookup_cache(int cache_type, ...)
{
	void* ret = nullptr;
	va_list argList;
	va_start(argList, cache_type);

	if(cache_type == D_CACHE) {
		ret = lookup_d_cache(&cache_array[cache_type], va_arg(argList, nameidata*));
	} else if(cache_type == I_CACHE) {
		ret = lookup_i_cache(&cache_array[cache_type], va_arg(argList, uint));
	} else if(cache_type == P_CACHE) {
		ret = lookup_p_cache(&cache_array[cache_type]);
	}

	va_end(argList);
	return ret;
}

dentry* lookup_d_cache(fscache* cache, const nameidata *nd)
{
	dentry *ret = nullptr;

	uint hash_value = hash_string(nd->nd_cur_name);
	list_head *head = &(cache->hash[hash_value]);
	list_head *p;
	list_for_each(p, head) {
		dentry* cur_entry = container_of(p, dentry, d_hash);
		if(cur_entry->d_parent == nd->nd_dentry 
			&& cur_entry->d_op->compare(cur_entry->d_name, nd->nd_cur_name) == 0) {
			ret = cur_entry;
			break;
		}
	}
	return ret;
}

inode* 	lookup_i_cache(fscache* cache, uint base_sec)
{
	inode *ret = nullptr;

	uint hash_value = hash_int(base_sec);
	list_head *head = &(cache->hash[hash_value]);
	list_head *p;
	list_for_each(p, head) {
		inode* cur_node = container_of(p, inode, i_hash);
		if(cur_node->i_ino == base_sec) {
			ret = cur_node;
			break;
		}
	}
	return ret;
}

fspage* lookup_p_cache(fscache* cache)
{

}

// 哈希函数
uint hash_string(qstr key)
{
	// ELF_hash
	uint hash = 0;
	uint x = 0;
	for(int i = 0; i < key.len; i++)
	{
		hash = (hash << 4) + key.name[i];
		if((x = hash & 0xF0000000) != 0)
		{
			hash ^= (x >> 24);
			hash &= ~x;
		}
	}

	return hash & HASH_MASK;
}

uint hash_int(uint key)
{
	return key & HASH_MASK;
}
