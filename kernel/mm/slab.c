#include "slab.h"
#include "buddy.h"

#include <driver/vga.h>
#include <intr.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

struct kmem_cache kmem_caches[KMEM_CACHE_NUM];
uint kmem_cache_size[KMEM_CACHE_NUM] = {8, 16, 32, 64,96, 128, 192, 256, 512, 1024, 1536, 2048};

void init_slab()
{
	for (uint i = 0; i < KMEM_CACHE_NUM; i++) {
		init_kmem_cache(kmem_caches + i, kmem_cache_size[i]);
	}
}

void init_kmem_cache(struct kmem_cache *cachep, uint size)
{
	cachep->object_size = upper_align(size, SLAB_ALIGN);
	cachep->size = upper_align(cachep->object_size + sizeof(struct list_head), SLAB_ALIGN);
	cachep->cpu.page = nullptr;
	cachep->capacity = (PAGE_SIZE - sizeof(struct slab_head)) / cachep->size;
	init_kmem_cache_node(&cachep->node);
}

void init_kmem_cache_node(struct kmem_cache_node *cache_nodep)
{
	INIT_LIST_HEAD(&cache_nodep->free);
	INIT_LIST_HEAD(&cache_nodep->full);
	INIT_LIST_HEAD(&cache_nodep->partial);
}

void *alloc_object(struct kmem_cache *cachep, struct slab_head *slabp)
{
	void *ret = nullptr;
	if (slabp->free_num == 1) {
		alloc_new_object_in_page(cachep, slabp);
	}
	if (slabp->free_num) {
		struct list_head *alloc_node = slabp->free_list.next;
		ret = (uchar*)alloc_node - cachep->object_size;
		list_del(alloc_node);
		slabp->free_num --;
		list_add(alloc_node, &slabp->alloc_list);
		slabp->alloc_num ++;
	}
	return ret;
}

void init_object_in_page(struct kmem_cache *cachep, struct page *pagep)
{
	struct slab_head *slabp = pagep->virtual;
	INIT_LIST_HEAD(&slabp->alloc_list);
	slabp->alloc_num = 0;
	INIT_LIST_HEAD(&slabp->free_list);
	uchar *first_free = (uchar *)slabp + sizeof(struct slab_head) + cachep->size;
	list_add_tail((struct list_head *)first_free, &slabp->free_list);
	slabp->free_num = 1;
}

void alloc_new_object_in_page(struct kmem_cache *cachep, struct slab_head *slabp)
{
	if (slabp->alloc_num + slabp->free_num < cachep->capacity) {
		uchar *next_free = (uchar *)slabp->free_list.prev + cachep->size;
		list_add_tail((struct list_head *)next_free, &slabp->free_list);
		slabp->free_num ++;
	}
}

void *kmem_cache_alloc(struct kmem_cache *cachep)
{
	void *ret = nullptr;

	struct page *pagep = cachep->cpu.page;
	if (pagep == 0 || (pagep && slab_is_full(cachep, (struct slab_head *)pagep->virtual))) {
		if (!list_empty(&cachep->node.partial)) {
			pagep = list_first_entry(&cachep->node.partial, struct page, list);
		} else if (!list_empty(&cachep->node.free)) {
			pagep = list_first_entry(&cachep->node.free, struct page, list);
		} else {
			pagep = __alloc_pages(&buddy_mm, 0);
			if (!pagep) {
				kernel_printf("No buddy page for slab!\n");
				return ret;
			} else {
				pagep->virtual = get_kernel_vaddr(get_page_paddr(pagep));
				pagep->used_info = BUDDY_SLAB;
				pagep->cachep = cachep;
				list_add(&pagep->list, &cachep->node.free);
			}
			init_object_in_page(cachep, pagep);
		}
		cachep->cpu.page = pagep;
	}
	struct slab_head *page_slab = pagep->virtual;
	int was_free = slab_is_empty(cachep, page_slab);

	ret = alloc_object(cachep, page_slab);
	if (slab_is_full(cachep, page_slab)) {
		list_del(&pagep->list);
		list_add(&pagep->list, &cachep->node.full);
	} else if (was_free) {
		list_del(&pagep->list);
		list_add(&pagep->list, &cachep->node.partial);
	}
	return ret;
}

void kmem_cache_free(struct kmem_cache *cachep, void *objp)
{
	struct slab_head *slabp = (struct slab_head *)lower_align((uint)objp, PAGE_SIZE);
	struct page *pagep = get_page_by_slab(slabp);
	struct list_head *obj_list = (struct list_head *)((uchar *)objp + cachep->object_size);
	int was_full = 0;
	if (slab_is_full(cachep, slabp)) {
		was_full = 1;
	}
	list_del(obj_list);
	slabp->alloc_num --;
	list_add(obj_list, &slabp->free_list);
	slabp->free_num ++;
	if (slabp->alloc_num == 0) {
		list_del(&pagep->list);
		list_add(&pagep->list, &cachep->node.free);
	} else if (was_full) {
		list_del(&pagep->list);
		list_add(&pagep->list, &cachep->node.partial);
	}
}

void kmem_cache_free_page(struct kmem_cache *cachep, void *pagevp)
{
	struct slab_head *slabp = pagevp;
	struct page *pagep = get_page_by_slab(slabp);
	if (slabp->alloc_num) {
		kernel_printf("Slab free an alloced page!\n");
		return;
	}
	list_del(&pagep->list);
	__free_pages(&buddy_mm, pagep);
}

void *kmalloc(uint size)
{
	int old = disable_interrupts();
	void *ret = nullptr;
	
	if (size <= kmem_cache_size[KMEM_CACHE_NUM-1]) {
		for (int i = 0; i < KMEM_CACHE_NUM; i++) {
			if (size <= kmem_cache_size[i]) {
				ret = kmem_cache_alloc(kmem_caches + i);
				break;
			}
		}
	} else {
		ret = alloc_pages(size);
		if (ret) {
			ret = get_kernel_vaddr(ret);
		}
	}
	if (old)
		enable_interrupts();
	return ret;
}

void kfree(void *objp)
{
	int old = disable_interrupts();
	struct slab_head *slabp = (struct slab_head *)lower_align((uint)objp, PAGE_SIZE);
	struct page *pagep = get_page_by_slab(slabp);
	if (pagep->used_info == BUDDY_SLAB) {
		kmem_cache_free(pagep->cachep, objp);
	} else {
		free_pages(get_kernel_paddr(objp));
	}
	if (old)
		enable_interrupts();
}

void print_slab_info()
{
	kernel_printf("<===== Slab Info =====>\n");

	for (int i = 0; i < KMEM_CACHE_NUM; i++) {
		struct kmem_cache *cachep = kmem_caches + i;
		kernel_printf("kemem_cache: %d\n", i);
		kernel_printf("size = %d, object_size = %d\n", cachep->size, cachep->object_size);
		kernel_printf("capacity = %d\n", cachep->capacity);
		int page_cnt = 0;
		int slab_alloc_cnt = 0;
		int slab_free_cnt = 0;
		struct page *pagep;
		list_for_each_entry(pagep, &cachep->node.full, list) {
			page_cnt ++;
			slab_alloc_cnt += cachep->capacity;
		}
		list_for_each_entry(pagep, &cachep->node.free, list) {
			page_cnt ++;
			slab_free_cnt += cachep->capacity;
		}
		list_for_each_entry(pagep, &cachep->node.partial, list) {
			kernel_printf("partial\n");
			page_cnt ++;
			struct slab_head *slabp = pagep->virtual;
			slab_alloc_cnt += slabp->alloc_num;
			slab_free_cnt += cachep->capacity - slabp->alloc_num;
		}
		kernel_printf("page_num = %d\n", page_cnt);
		kernel_printf("total_slab_num = %d\n", page_cnt * cachep->capacity);
		kernel_printf("alloc_num = %d\n", slab_alloc_cnt);
		kernel_printf("free_num = %d\n", slab_free_cnt);
	}

	kernel_printf("<===== Slab Info End =====>\n");
}

void test_slab()
{
	// void **addrs = (void **)kmalloc(4 * 555);
	for (int i = 0; i < 2; i++) {
		void *addr = kmalloc(8);
		kernel_printf("%x\n", addr);
		kfree(addr);
	}
	for (int i = 0; i < 2; i++) {
		void *addr = kmalloc(16);
		kernel_printf("%x\n", addr);
		kfree(addr);
	}
	void *addr = kmalloc(8);
	void *addr2 = kmalloc(16);
	// kernel_printf("%x\n", addrs);
	// for (int i = 1; i < 255; i++) {
	// 	kfree(addrs[i]);
	// }
	print_slab_info();
	// print_buddy_info();
	kfree(addr);
	kfree(addr2);
	// kernel_printf("%x\n", addr);

	// kfree(addrs);
}

#pragma GCC pop_options