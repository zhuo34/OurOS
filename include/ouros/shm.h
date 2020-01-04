#ifndef OUROS_SHM_H
#define OUROS_SHM_H

#include <linux/list.h>

struct shm_struct {
	uint pfn_start;
	uint pfn_num;
	struct list_head list;
};

struct shm_pool_struct {
	uint alloc_num;
	struct list_head alloc_list;
};

void init_shm_pool();

struct shm_struct *shmget(uint size);
void shmat(struct shm_struct *shm, void *vaddr);

#endif // OUROS_SHM_H
