#include "pageswap.h"

#include <driver/vga.h>
#include <ouros/fs/fs.h>

struct page_pool_struct page_pool;

void init_page_pool()
{
	page_pool.page_num = 0;
	page_pool.next_free = 1;
	for (int i = 0; i < MACHINE_PAGE_NUM; i++) {
		page_pool.entry[i] = 0;
		page_pool.block[i] = nullptr;
	}
}

uint write_page_to_disk(void *vaddr, uint block_num)
{
	if (!block_num) {
		/*
		* allocate a new block for this page
		*/
		block_num = page_pool.next_free;
		uint next_next_free = page_pool.entry[page_pool.next_free - 1];
		if (next_next_free) {
			page_pool.next_free = next_next_free;
		} else {
			page_pool.next_free ++;
			page_pool.page_num ++;
		}
	}

	// kernel_printf("save %d\n", block_num);
	/**
	 * TODO: write to disk
	 */
	// page_pool.block[block_num-1] = kmalloc(PAGE_SIZE);
	// kernel_memcpy(page_pool.block[block_num-1], vaddr, PAGE_SIZE);
	file *fp = fs_open("/pages", F_MODE_READ_WRITE);
	fs_seek(fp, SEEK_HEAD, (block_num - 1) * PAGE_SIZE);
	fs_write(fp, vaddr, PAGE_SIZE);
	fs_close(fp);

	return block_num;
}

void load_page_from_disk(void *vaddr, uint block_num)
{
	// kernel_printf("load %d\n", block_num);
	if (block_num) {
		// kernel_memcpy(vaddr, page_pool.block[block_num-1], PAGE_SIZE);
		file *fp = fs_open("/pages", F_MODE_READ);
		fs_seek(fp, SEEK_HEAD, (block_num - 1) * PAGE_SIZE);
		fs_read(fp, vaddr, PAGE_SIZE);
		fs_close(fp);
	}
}

void delete_page_on_disk(uint block_num)
{
	kfree(page_pool.block[block_num-1]);
	page_pool.entry[block_num-1] = page_pool.next_free;
	page_pool.next_free = block_num;
}