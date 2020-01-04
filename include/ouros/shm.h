#ifndef OUROS_SHM_H
#define OUROS_SHM_H

#define MAX_SHM_NUM 8

struct shm_struct {
	uint pfn_start;
	uint pfn_num;
};

// struct shm_pool_struct {
// 	// uint alloc_num;
// 	struct shm_struct *alloc_list[MAX_SHM_NUM];
// };

void init_shm_pool();

int shmget(int key, uint size);
void shmat(int shmid, void *vaddr);

#endif // OUROS_SHM_H
