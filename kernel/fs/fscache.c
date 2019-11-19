#include "fscache.h"

#include <ouros/error.h>

static fscache *dcache;
static fscache *icache;

int init_fscache()
{
	int error = NO_ERROR;

	dcache = create_fscache(DCACHE_CAPACITY);
	if(!dcache)
		goto err;
	
	icache = create_fscache(ICACHE_CAPACITY);
	if(!icache)
		goto err;

	goto exit;

err:
	kfree_fake(dcache);
	kfree_fake(icache);
	error = -ERROR_NO_MEMORY;
exit:
	return error;
}

fscache* create_fscache(int capacity)
{
	fscache *ret = (fscache*)kmalloc_fake(sizeof(fscache));
	if(ret) {
		ret->size = 0;
		ret->capacity = capacity;
		ret->hash = (list_head*)kmalloc_fake(sizeof(list_head) * HASH_SIZE);
		for(int i=0; i<HASH_SIZE; i++) {
			INIT_LIST_HEAD(&ret->hash[i]);
		}
		INIT_LIST_HEAD(&ret->LRU);
	}

	return ret;
}

dentry* lookup_dcache(const nameidata *nd)
{
	dentry *ret = nullptr;

	uint hash_value = hash_string(nd->nd_cur_name);
	list_head *head = &dcache->hash[hash_value];
	list_head *p;
	list_for_each(p, head) {
		dentry* cur_entry = container_of(p, dentry, d_hash);
		if(cur_entry->d_parent == nd->nd_dentry 
			&& cur_entry->d_op->compare(cur_entry->d_name, nd->nd_cur_name) == 0) {
			ret = cur_entry;
			break;
		}
	}

	// 找到了则更新hash表和LRU链表
	if(ret) {
		list_del(ret);
		list_add(ret, &dcache->hash[hash_value]);
		list_del(ret);
		list_add(ret, &dcache->LRU);
	}

	return ret;
}

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