#include "fscache.h"

#include <ouros/error.h>

static fscache *dcache;
static fscache *icache;

int init_fscache()
{
	int error = NO_ERROR;

	dcache = (fscache*)kmalloc_fake(sizeof(fscache));
	if(!dcache)
		goto err;
	dcache->size = 0;
	dcache->capacity = DCACHE_CAPACITY;
	dcache->hash = (list_node*)kmalloc_fake(sizeof(list_node) * HASH_SIZE);
	for(int i=0; i<HASH_SIZE; i++)
		INIT_LIS
	
	icache = (fscache*)kmalloc_fake(sizeof(fscache));
	if(!icache)
		goto err;

err:
	kfree_fake(dcache);
	kfree_fake(icache);
	error = -ERROR_NO_MEMORY;
exit:
	return error;
}