#include "shm.h"

#include <ouros/mm.h>
#include <ouros/buddy.h>
#include <ouros/vm.h>

struct shm_pool_struct shm_pool;

void init_shm_pool()
{
	shm_pool.alloc_num = 0;
	INIT_LIST_HEAD(&shm_pool.alloc_list);
}

struct shm_struct *shmget(uint size)
{
	if (!size)
		return nullptr;
	void *paddr = alloc_pages(size);
	if (!paddr) {
		return nullptr;
	}
	struct shm_struct *shm = (struct shm_struct *)kmalloc(sizeof(struct shm_struct));
	shm->pfn_start = get_pgn(get_page_by_paddr(paddr));
	shm->pfn_start = upper_align(size, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
	list_add_tail(&shm->list, &shm_pool.alloc_list);
	shm_pool.alloc_num ++;
}

void shmat(struct shm_struct *shm, void *vaddr)
{
	void *p = (void *)lower_align((uint)vaddr, 1 << PAGE_SHIFT);
	for (int i = 0; i < shm->pfn_num; i++) {
		get_pte(mm_current->pgd, p)->tlb_entry.reg.PFN = shm->pfn_start + i;
		get_pte(mm_current->pgd, p)->tlb_entry.reg.G = 0;
		get_pte(mm_current->pgd, p)->tlb_entry.reg.D = 1;
		get_pte(mm_current->pgd, p)->tlb_entry.reg.V = 1;
		p += i * PAGE_SIZE;
	}
}
