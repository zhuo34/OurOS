#ifndef SLAB_H
#define SLAB_H

#include <ouros/slab.h>

#define get_page_by_slab(slabp) (get_page_by_paddr(get_kernel_paddr(slabp)))
#define slab_is_full(cachep, slabp) ((slabp)->alloc_num == (cachep)->capacity)
#define slab_is_empty(cachep, slabp) ((slabp)->alloc_num == 0)

void init_kmem_cache(struct kmem_cache *cachep, uint size);
void init_kmem_cache_node(struct kmem_cache_node *cache_nodep);

void init_object_in_page(struct kmem_cache *cachep, struct page *pagep);
void *alloc_object(struct kmem_cache *cachep, struct slab_head *slabp);
void alloc_new_object_in_page(struct kmem_cache *cachep, struct slab_head *slabp);

void *kmem_cache_alloc(struct kmem_cache *cachep);
void kmem_cache_free(struct kmem_cache *cachep, void *objp);
void kmem_cache_free_page(struct kmem_cache *cachep, void *pagevp);

#endif // SLAB_H