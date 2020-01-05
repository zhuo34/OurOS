#include "shm.h"

#include <ouros/mm.h>
#include <ouros/buddy.h>
#include <ouros/vm.h>

struct shm_struct *shm_pool[MAX_SHM_NUM];

void init_shm_pool()
{
	kernel_memset(shm_pool, 0, sizeof(struct shm_struct *) * MAX_SHM_NUM);
}

/**
 * @brief Get shared memory.
 * @param key	shared memory key
 * @param size	shared memory size
 * @return shared memory id
 */
int shmget(int key, uint size)
{
	if (!size || key < 0 || key >= MAX_SHM_NUM)
		return -1;

	if (!shm_pool[key]) {
		/**
		 * If key is not registered, allocate new shared memory.
		 */
		void *paddr = alloc_pages(size);
		if (!paddr)
			return -1;
		struct shm_struct *shm = (struct shm_struct *)kmalloc(sizeof(struct shm_struct));
		/**
		 * Shared memory needs physical address, while kmalloc() return virtual address,
		 * which is in kernel segment in MIPS architecture
		 * and therefore is easy to convert to phsical address as follows.
		 * 
		 * 	  kmalloc virtual address:	???x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
		 * => physical address:			000x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx
		 */
		shm->pfn_start = get_pgn(get_page_by_paddr(paddr));
		shm->pfn_num = upper_align(size, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
		shm_pool[key] = shm;
	}
	return key;
}

/**
 * @brief Bind shared memory.
 * @param shmid		shared memory id
 * @param vaddr		virtual address to bind
 */
void shmat(int shmid, void *vaddr)
{
	if (shmid < 0 || shmid >= MAX_SHM_NUM || !shm_pool[shmid])
		return;
	void *p = (void *)lower_align((uint)vaddr, 1 << PAGE_SHIFT);
	for (int i = 0; i < shm_pool[shmid]->pfn_num; i++) {
		/**
		 * Bind virtual address onto shared physical memory
		 * by set the correspond page table.
		 */
		pte_t *pte = get_pte(mm_current->pgd, p);
		pte->tlb_entry.reg.PFN = shm_pool[shmid]->pfn_start + i;
		pte->tlb_entry.reg.G = 0;
		pte->tlb_entry.reg.D = 1;
		pte->tlb_entry.reg.V = 1;
		p += i * PAGE_SIZE;
	}
}
