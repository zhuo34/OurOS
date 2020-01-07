#ifndef OUROS_SLAB_H
#define OUROS_SLAB_H

#include <ouros/mm.h>
#include <linux/list.h>

#define SLAB_ALIGN 4
#define KMEM_CACHE_NUM 12

/**
 * struct slab_head defines the beginning of a page, leading page internal structures,
 * which gives slab system the ability to delicately manage physical memory with size < PAGE_SIZE.
 * However, as we all know, OS use virtual address to access memory,
 * which means if we want to maintain this internal structures, we need get a virtual address of this page.
 * 
 * Fortunately it's easy to get a virtual address:
 * the machine physical memory size is 512 M, which is equal to kernel unmapped segment (see vm.h),
 * which in MIPS refers to 0x8000_0000 - 0x9FFF_FFFF, 512 MB,
 * therefore any physical address from buddy can be directly mapped onto kernel unmapped segment,
 * whose phsical addresses are easy to convert to virtual address as follows,
 * which is what macro function get_kernel_vaddr() does.
 * 
 *    physical address:			000x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
 * => kmalloc virtual address:	010x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
 * 
 * Page Internal Structure:
 *                 ++++++++++++++++++++ <= pagep->virtual (void *), slabp (struct slab_head *)
 *                 +    slab head     +
 *                 +------------------+
 *                 +  two link list   +
 *                 ++++++++++++++++++++ <= allocated objp (virtual address)
 *                 +      object      +
 *                 +------------------+
 *                 +  link list node  +
 *                 ++++++++++++++++++++
 *                 +        .         +
 *                 +        .         +
 *                 +        .         +
 *                 ++++++++++++++++++++
 *                 +      object      +
 *                 +------------------+
 *                 +  link list node  +
 *                 ++++++++++++++++++++
 */
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

void test_slab();

void print_slab_info();

#endif // OUROS_SLAB_H
