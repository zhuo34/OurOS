#include "shm.h"

#include <ouros/mm.h>
#include <ouros/buddy.h>
#include <ouros/vm.h>

struct shm_struct *shm_pool[MAX_SHM_NUM];

void init_shm_pool()
{
	kernel_memset(shm_pool, 0, sizeof(struct shm_struct *) * MAX_SHM_NUM);
}

int shmget(uint size)
{
	int ret = -1;
	if (!size)
		return ret;
	for (int i = 0; i < MAX_SHM_NUM; i++) {
		if (!shm_pool[i]) {
			ret = i;
			break;
		}
	}
	if (ret == -1) return ret;
	void *paddr = alloc_pages(size);
	if (!paddr) {
		return ret;
	}
	struct shm_struct *shm = (struct shm_struct *)kmalloc(sizeof(struct shm_struct));
	shm->pfn_start = get_pgn(get_page_by_paddr(paddr));
	shm->pfn_start = upper_align(size, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
	shm_pool[ret] = shm;
	return ret;
}

void shmat(int shmid, void *vaddr)
{
	if (shmid < 0 || shmid >= MAX_SHM_NUM || !shm_pool[shmid])
		return;
	void *p = (void *)lower_align((uint)vaddr, 1 << PAGE_SHIFT);
	for (int i = 0; i < shm_pool[shmid]->pfn_num; i++) {
		get_pte(mm_current->pgd, p)->tlb_entry.reg.PFN = shm_pool[shmid]->pfn_start + i;
		get_pte(mm_current->pgd, p)->tlb_entry.reg.G = 0;
		get_pte(mm_current->pgd, p)->tlb_entry.reg.D = 1;
		get_pte(mm_current->pgd, p)->tlb_entry.reg.V = 1;
		p += i * PAGE_SIZE;
	}
}
