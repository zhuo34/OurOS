#ifndef OUROS_SLAB_H
#define OUROS_SLAB_H

#include <ouros/mm.h>
#include <ouros/list.h>

#define SLAB_ALIGN 0x100
#define KMEM_CACHE_NUM 12

struct slab_head {
	struct list_head alloc_list;
	struct list_head free_list;
	uint alloc_num;
	uint free_num;
};

struct kmem_cache_node {
    struct list_head partial;
    struct list_head full;
	struct list_head free;
};

struct kmem_cache_cpu {
	struct page *page;
};

struct kmem_cache {
	uint size;
	uint object_size;
	uint capacity;
	struct kmem_cache_node node;
	struct kmem_cache_cpu cpu;
};

void init_slab();

void *kmalloc(uint size);

void kfree(void *objp);

#endif // OUROS_SLAB_H
