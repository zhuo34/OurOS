#include "slab.h"
#include "buddy.h"

#include <driver/vga.h>
#include <intr.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

struct kmem_cache kmem_caches[KMEM_CACHE_NUM];
uint kmem_cache_size[KMEM_CACHE_NUM] = {8, 16, 32, 64,96, 128, 192, 256, 512, 1024, 1536, 2048};

/**
 * @brief Initialize slab system.
 */
void init_slab()
{
	for (uint i = 0; i < KMEM_CACHE_NUM; i++) {
		init_kmem_cache(kmem_caches + i, kmem_cache_size[i]);
	}
}

/**
 * @brief Initialize a kernel memory cache.
 * @param cachep	pointer of cache
 * @param size		size of this cache
 */
void init_kmem_cache(struct kmem_cache *cachep, uint size)
{
	cachep->object_size = upper_align(size, SLAB_ALIGN);
	cachep->size = upper_align(cachep->object_size + sizeof(struct list_head), SLAB_ALIGN);
	cachep->cpu.page = nullptr;
	cachep->capacity = (PAGE_SIZE - sizeof(struct slab_head)) / cachep->size;
	init_kmem_cache_node(&cachep->node);
}

/**
 * @brief Initialize 3 linked lists of kernel memory cache.
 * @param cache_nodep	pointer of cache node
 */
void init_kmem_cache_node(struct kmem_cache_node *cache_nodep)
{
	INIT_LIST_HEAD(&cache_nodep->free);
	INIT_LIST_HEAD(&cache_nodep->full);
	INIT_LIST_HEAD(&cache_nodep->partial);
}

/**
 * @brief Allocate an object from a page with size same as given cachep.
 * @param cachep	cache pointer
 * @param slabp		slab pointer
 * @return allocated virtual address
 */
void *alloc_object(struct kmem_cache *cachep, struct slab_head *slabp)
{
	void *ret = nullptr;
	/**
	 * If there remains only 1 slab in this page,
	 * try to allocate a new object, that means add the new object
	 * into free list of this page.
	 */
	if (slabp->free_num == 1) {
		alloc_new_object_in_page(cachep, slabp);
	}
	/**
	 * If there remains slabs in this page,
	 * set values for output.
	 */
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

/**
 * @brief Initialize a cached page, that is, configure slabs in this page.
 * @param cachep	pointer of cache
 * @param pagep		pointer of physical page
 */
void init_object_in_page(struct kmem_cache *cachep, struct page *pagep)
{
	struct slab_head *slabp = pagep->virtual;
	/**
	 * Configure slab lists.
	 */
	INIT_LIST_HEAD(&slabp->alloc_list);
	slabp->alloc_num = 0;
	INIT_LIST_HEAD(&slabp->free_list);
	uchar *first_free = (uchar *)slabp + sizeof(struct slab_head) + cachep->size;
	list_add_tail((struct list_head *)first_free, &slabp->free_list);
	slabp->free_num = 1;
}

/**
 * @brief Allocate a new object in a page.
 * @param cachep	pointer of cache
 * @param pagep		pointer of physical page
 */
void alloc_new_object_in_page(struct kmem_cache *cachep, struct slab_head *slabp)
{
	/**
	 * If this page is full, directly return;
	 * otherwise, configure a new object, and append it on slab list.
	 */
	if (slabp->alloc_num + slabp->free_num < cachep->capacity) {
		uchar *next_free = (uchar *)slabp->free_list.prev + cachep->size;
		list_add_tail((struct list_head *)next_free, &slabp->free_list);
		slabp->free_num ++;
	}
}

/**
 * @brief Allocate a object from cache.
 * @param cachep cache
 * @return allocated virtual address
 */
void *kmem_cache_alloc(struct kmem_cache *cachep)
{
	void *ret = nullptr;

	struct page *pagep = cachep->cpu.page;
	if (pagep == 0 || (pagep && slab_is_full(cachep, (struct slab_head *)pagep->virtual))) {
		/**
		 * If current page of this cache is NULL or full,
		 * we need to put this page in full list and get a non-full page of this cache.
		 */
		if (!list_empty(&cachep->node.partial)) {
			/* Get a page from partial list. */
			pagep = list_first_entry(&cachep->node.partial, struct page, list);
		} else if (!list_empty(&cachep->node.free)) {
			/* Get a page from free list. */
			pagep = list_first_entry(&cachep->node.free, struct page, list);
		} else {
			/* Get a page from buddy system. */
			pagep = __alloc_pages(&buddy_mm, 0, BUDDY_SLAB);
			if (!pagep) {
				kernel_printf("No buddy page for slab!\n");
				return ret;
			} else {
				/**
				 * Initialize the page fit to size of this cache.
				 * 1. set virtual address of the page from buddy system,
				 *    which is used for slab system to manage page internal structure
				 * 2. set used information
				 * 3. set cache pointer
				 * 4. add this page into free list of this cache
				 * 5. initialize the slab head of this page
				 */
				pagep->virtual = get_kernel_vaddr(get_page_paddr(pagep));
				pagep->used_info = BUDDY_SLAB;
				pagep->cachep = cachep;
				list_add(&pagep->list, &cachep->node.free);
				init_object_in_page(cachep, pagep);
			}
		}
		/* Reset current page. */
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

/**
 * @brief Free memory allocated from slab system.
 * @param cachep	cache pointer
 * @param objp		allocated object virtual address
 */
void kmem_cache_free(struct kmem_cache *cachep, void *objp)
{
	struct slab_head *slabp = (struct slab_head *)lower_align((uint)objp, PAGE_SIZE);
	struct page *pagep = get_page_by_slab(slabp);
	struct list_head *obj_list = (struct list_head *)((uchar *)objp + cachep->object_size);
	int was_full = 0;
	if (slab_is_full(cachep, slabp)) {
		was_full = 1;
	}
	/**
	 * Maintain the internal structure:
	 * delete this object from allocated list of corresponed slab head,
	 * then add it in free list.
	 */
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

/**
 * @brief Slab system return a page allocated from buddy system.
 * @param cachep	cache
 * @param pagevp	virtual address of a page
 */
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

/**
 * @brief Allocate memroy of given size in kernel.
 * @param size	memory size
 * @return allocated virtual address
 */
void *kmalloc(uint size)
{
	int old = disable_interrupts();
	void *ret = nullptr;
	
	if (size <= kmem_cache_size[KMEM_CACHE_NUM-1]) {
		/**
		 * If size is less than the largest cache,
		 * allocate from slab system.
		 */
		for (int i = 0; i < KMEM_CACHE_NUM; i++) {
			if (size <= kmem_cache_size[i]) {
				ret = kmem_cache_alloc(kmem_caches + i);
				break;
			}
		}
	} else {
		/**
		 * If size is larger than the largest cache,
		 * allocate from buddy system.
		 */
		ret = alloc_pages(size);
		if (ret) {
			/* Get virtual address. */
			ret = get_kernel_vaddr(ret);
		}
	}
	if (old)
		enable_interrupts();
	return ret;
}

/**
 * @brief Free memroy allocated by kmalloc().
 * @param objp	virtual address
 */
void kfree(void *objp)
{
	int old = disable_interrupts();
	/**
	 * 1. get slab head address (which is also the page virtual address) from object virtual address
	 * 2. get page struct of this address
	 * 3. check the used information of this page
	 *    a) slab: request slab system to free it
	 *    b) buddy: request buddy system to free it
	 */
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
			page_cnt ++;
			struct slab_head *slabp = pagep->virtual;
			slab_alloc_cnt += slabp->alloc_num;
			slab_free_cnt += cachep->capacity - slabp->alloc_num;
		}
		if (page_cnt) {
			kernel_printf("%d:", cachep->object_size);
			kernel_printf("\t%d", page_cnt);
			kernel_printf("\t%d", page_cnt * cachep->capacity);
			kernel_printf("\t%d", slab_alloc_cnt);
			kernel_printf("\t%d\n", slab_free_cnt);
		}
	}

	kernel_printf("<===== Slab Info End =====>\n");
}

void test_slab()
{
	void **addrs = (void **)kmalloc(sizeof(void *) * 158); // 632
	for (int i = 0; i < 16; i++) {
		addrs[i] = kmalloc(256);
	}
	print_slab_info();
	for (int i = 0; i < 16; i++) {
		kfree(addrs[i]);
	}
	kfree(addrs);
	print_slab_info();
}

#pragma GCC pop_options