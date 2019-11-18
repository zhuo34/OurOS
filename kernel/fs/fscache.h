#ifndef _FS_FSCACHE_H_
#define _FS_FSCACHE_H_

#include <ouros/fs/fscache.h>

#define HASH_SIZE 			32
#define HASH_MASK 			(HASH_SIZE - 1)

#define DCACHE_CAPACITY 	32
#define ICACHE_CAPACITY 	32

typedef struct _fscache
{
	uint 			size;
	uint 			capacity;
	list_node 		LRU;
	list_node* 		hash;
}fscache;

#endif