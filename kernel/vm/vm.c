#include "vm.h"
#include "pageswap.h"

#include <tlb.h>
#include <ouros/pc.h>
#include <ouros/mm.h>
#include <ouros/utils.h>
#include <ouros/buddy.h>
#include <driver/vga.h>
#include <ouros/error.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

struct mm_struct *mm_current;
int page_fault_debug = 0;

void test_page_fault(int val)
{
	kernel_printf(">>>>>>> start test page fault\n");
	if (page_fault_debug) {
		mm_current = create_mm_struct(0);
	}
	/**
	 * In page fault debug, MAX_PFN_PER_PROCESS is set to 3.
	 * Use 4 test virtuak addressess here for testing swap page.
	 */
	int *test_vaddrs[4];
	for (int i = 0; i < 4; i++) {
		test_vaddrs[i] = (int *)(0x08000000 + 0x00004000 * i);	// 0x0800_?000
		kernel_printf("access %x\n", test_vaddrs[i]);
		int a = val + i;
		*(test_vaddrs[i]) = a;							// directly access virtual address
		kernel_printf("test %d\n", *(test_vaddrs[i]));	// test if page fault done
	}
	if (page_fault_debug) {								// for testing page fault swap page
		/* access 0x0800_0000 which is swapped out*/
		int * addr = (int *)0x08000000;
		kernel_printf(">>>>> access %x\n", addr);
		int a = *(addr);
		kernel_printf("test %d\n", a);
		kernel_printf("<<<<< access %x end\n", addr);
		free_mm_struct(mm_current);
	}
	kernel_printf("<<<<<<< end test page fault\n");
}

/**
 * @brief Create a new mm_struct.
 * @param asid	process asid
 * @return mm_struct
 */
struct mm_struct *create_mm_struct(uint asid)
{
	struct mm_struct *mm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct));
	mm->pgd = create_pt();
	mm->pf_num = 0;
	mm->asid = asid;
	// mm->count = 1;
	INIT_LIST_HEAD(&mm->fifo);
	return mm;
}

/**
 * @brief Create first level page table.
 * @return pgd pointer
 */
pgd_t *create_pt()
{
	pgd_t *ret = (pgd_t *)kmalloc(sizeof(pgd_t) * PGD_NUM);
	kernel_memset(ret, 0, sizeof(pgd_t) * PGD_NUM);
	return ret;
}

/**
 * @brief Get pte of a virtual address.
 * @param pgd	page table
 * @param vaddr	virtual addrss
 * @return pte pointer
 */
pte_t *get_pte(pgd_t *pgd, void *vaddr)
{
	uint addr = (uint) vaddr;
	uint pgd_idx = addr >> 22;
	if (!pgd[pgd_idx]) {
		/**
		 * If this second level page table is not allocated,
		 * then allocate memory at this first access time.
		 */
		pgd[pgd_idx] = (pte_t *)kmalloc(sizeof(pte_t) * PTE_NUM);
		kernel_memset(pgd[pgd_idx], 0, sizeof(pte_t) * PTE_NUM);
	}
	uint pte_idx = (addr << 10) >> 22;
	return &(pgd[pgd_idx][pte_idx]);
}

/**
 * @brief Check page table entry valid field.
 * @param pte	page table entry
 * @return valid or not
 */
bool pte_is_valid(pte_t *pte)
{
	return pte->tlb_entry.reg.V;
}

/**
 * @brief Get tlb information of a page table entry.
 * @param pte	page table entry
 * @return tlb information
 */
u32 get_pte_tlb_entry(pte_t *pte)
{
	return pte->tlb_entry.value;
}

/**
 * @brief Set tlb information of a page table entry.
 * @param pte	page table entry
 * @param value	value
 */
void set_pte_tlb_entry(pte_t *pte, u32 value)
{
	pte->tlb_entry.value = value;
}

/**
 * @brief [Unimplemented] Check if a virtual address is in a virtal memory space.
 * @param mm		mm_struct
 * @param vaddr		virtual address
 * @return check result
 */
int find_vma(struct mm_struct *mm, void *vaddr)
{
	return 1;
}

/**
 * @brief Do page fault for a virtual address.
 * @param vaddr		virtual address
 */
void do_pagefault(void *vaddr)
{
	int vma = find_vma(mm_current, vaddr);
	if (vma) {
		pte_t *pte = get_pte(mm_current->pgd, vaddr);
		/**
		 * If this virtual address is in vma
		 * 
		 * Need to reallocate a physical page frame for this virtual page.
		 * Swapping page frame is needed when there is no free physical page frame
		 * from BUDDY system.
		 */
		void *paddr = nullptr;
		uint max = MAX_PFN_PER_PROCESS;
		if (page_fault_debug)
			max = 3;
		if (mm_current->pf_num < max) {
			paddr = alloc_one_page();
			if (paddr) {
				/**
				 * If there is free page frame,
				 * 1. set virtual address of this page
				 * 2. add this page to FIFO list
				 * 3. pfn_num of this process increament
				 */
				struct page *pagep = get_page_by_paddr(paddr);
				pagep->virtual = vaddr;
				list_add_tail(&pagep->list, &mm_current->fifo);
				mm_current->pf_num ++;
			} else {
				/**
				 * If there is no free page frame
				 * Swap page frame
				 */
				paddr = swap_one_page(mm_current, vaddr);
			}
		} else {
			/**
			 * Swap page frame from this process
			 */
			paddr = swap_one_page(mm_current, vaddr);
		}
		if (page_fault_debug)
			kernel_printf("mapped phy addr: %x\n", paddr);
		uint lo = (((uint)paddr >> PAGE_SHIFT) << 6) | 0x1f;
		set_pte_tlb_entry(pte, lo);
	} else {
		/**
		 * 2. [Unimplemented] this virtual address is not in vma
		 * TODO: kill the process
		 */
	}
}

/**
 * @brief Swap one page.
 * @param mm		mm_struct
 * @param vaddr		virtual address
 * @return victim physical address
 */
void *swap_one_page(struct mm_struct *mm, void *vaddr)
{
	struct page *pagep = list_first_entry(&mm->fifo, struct page, list);
	void *vaddr_out = pagep->virtual;
	pte_t *pte = get_pte(mm->pgd, vaddr_out);
	void *paddr = (void *) (pte->tlb_entry.reg.PFN << PAGE_SHIFT);
	if (page_fault_debug) {
		kernel_printf("swap out vaddr %x\n", vaddr_out);
	}

	// kernel_printf("new block num: %d\n", write_page_to_disk(vaddr_out, pte->info));
	pte->info = write_page_to_disk(vaddr_out, pte->info);
	load_page_from_disk(vaddr_out, get_pte(mm->pgd, vaddr)->info);
	/**
	 * 1. tlb reset
	 * 2. delete the page swapped out from the fifo
	 * ! Attention:
	 * In MIPS architecture, nesting tlb miss will return to the origin EPC. 
	 * So put 'delete' at last in case:
	 * another tlb miss is triggered in 'write_page_to_disk' before,
	 * due to the victim page virtual address is not in tlb.
	 */
	pte->tlb_entry.reg.V = 0;
	tlb_reset(mm, vaddr_out);

	pagep->virtual = vaddr;
	list_del(&pagep->list);
	list_add_tail(&pagep->list, &mm->fifo);
	return paddr;
}

/**
 * @brief Get EntryHi of a virtual address.
 * @param vaddr		virtual address
 * @param asid		process asid
 * @return EntryHi value
 * 
 * EntryHi is a CP0 register in MIPS, it is used for operating tlb,
 * in tlbw, tlbp, etc, where EntryHi is referred as input or search key.
 * 
 * See ouros/arch/mips/tlb.c.
 */
u32 get_entry_hi(void *vaddr, uint asid)
{
	union EntryHi entry_hi;
	entry_hi.value = 0;
	entry_hi.reg.VPN2 = (uint)vaddr >> 13;
	entry_hi.reg.ASID = asid;
	return entry_hi.value;
}

/**
 * @brief Release memory allocated in a virtual memory space.
 * @param mm	mm_struct
 */
void free_mm_struct(struct mm_struct *mm)
{
	struct list_head *p = mm->fifo.next;
	while (p != &mm->fifo) {
		struct list_head *next = p->next;
		struct page *pagep = list_entry(p, struct page, list);
		pte_t *pte = get_pte(mm->pgd, pagep->virtual);
		pte->tlb_entry.reg.V = 0;
		tlb_reset(mm, pagep->virtual);
		if (page_fault_debug)
			kernel_printf("free phy addr: %x\n", get_page_paddr(pagep));
		list_del(p);
		free_pages(get_page_paddr(pagep));
		p = next;
	}

	for (int i = 0; i < PGD_NUM; i++) {
		if (mm->pgd[i]) {
			kfree(mm->pgd[i]);
		}
	}
	kfree(mm->pgd);
	kfree(mm);
}

/**
 * @brief [Bug] Load a file and map it in user space.
 * @param file_name		file name
 * @return load target virtual address
 */
void *mmap(const char *file_name)
{
	void *ret = nullptr;
	file *fp = fs_open(file_name, F_MODE_READ);
	if (IS_ERR_PTR(fp)) {
		kernel_printf("File Error!\n");
		return ret;
	}
	uchar *buf = (uchar*)0x1000;
	int cnt = 0;
	while (!eof(fp)) {
		fs_read(fp, buf + cnt++, 1);
	}
	fs_close(fp);
	ret = (void *)buf;
	return ret;
}

#pragma GCC pop_options
