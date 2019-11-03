#include "fs.h"

#include <driver/sd.h>

#include <ouros/log.h>
#include <ouros/error.h>
#include <assert.h>

static 	MBR 	*sdCard_MBR;

static Byte malloc_buffer[2048];
static int malloc_index = 0;

void* kmalloc(uint size) {
	Byte* ret = malloc_buffer + malloc_index;
	malloc_index += size;
	return ret; 
}
void  kfree(void* p) {

}

void init_fs()
{
	int err = NO_ERROR;

	err = read_MBR();
	if(err) {
		log(LOG_FAIL, "Read MBR");
		return;
	}
	log(LOG_OK, "Read MBR");

	// for(int i=0; i<sdCard_MBR->par_count; i++) {
	// 	sdCard_MBR->par_fs_system[i]->getSuperBlock;
	// 	superblock->init(baseaddr)
	// }
}

int read_MBR()
{
			sdCard_MBR 		= nullptr;
	Byte* 	buf_MBR 		= nullptr;

	int 	error 			= NO_ERROR;

	// 申请SD卡的MBR结构体
    sdCard_MBR = (MBR*)kmalloc(sizeof(MBR));
    if (sdCard_MBR == nullptr) {
		error = ERROR_NO_MEMORY;
		goto err;
	}
	
	// 申请buffer读取MBR数据
	buf_MBR = (Byte*)kmalloc(SECTOR_SIZE);
	if (buf_MBR == nullptr) {
		error = ERROR_NO_MEMORY;
		goto err;
	}
	kernel_memset_uint(buf_MBR, 0, SECTOR_SIZE);

	// 从SD卡读取MBR数据
	if (!sd_read_block(buf_MBR, 0, 1)) {
		error = ERROR_READ_MBR;
		goto err;
	}

	// 解析MBR数据
	// 主引导记录占446字节，之后的数据为硬盘分区表(DPT) Disk Partition Table
	DPT *curDPT = (DPT*)(buf_MBR + 446);
	int cnt;
	for(cnt = 0; cnt < MAX_PAR_COUNT; cnt ++) {
		DWord base_addr = read_unaligned(&curDPT->dpt_base_addr, sizeof(DWord));
		if(!base_addr) 
			break;
		kernel_printf("dpt_base_addr: %d\n", base_addr);
		
		sdCard_MBR->par_base_addr[cnt] = base_addr;
		if(error = get_fs_type(&sdCard_MBR->par_fs_type[cnt], curDPT->dpt_systemID)) {
			goto err;
		}
		curDPT ++;
	}
	sdCard_MBR->par_count = cnt;
	goto exit;

err:
	kfree(sdCard_MBR);
exit:
	kfree(buf_MBR);
	return error;

}

DWord read_unaligned(void *addr, uint size)
{
	kernel_assert(size > 0 && size <= 4, "read_unaligned");

	DWord ret = 0;
	kernel_memcpy(&ret, addr, (int)size);
	return ret;
}

int get_fs_type(FS_Type* const type, Byte systemID)
{
	int error = NO_ERROR;
	if(systemID == SYSTEM_ID_FAT32) {
		type->name = "fat32";
	} else if(systemID == SYSTEM_ID_EXT2) {
		type->name = "ext2";
	} else if(systemID == SYSTEM_ID_NTFS) {
		type->name = "ntfs";
	} else {
		type->name = "unknown";
		error = ERROR_UNKNOWN_FS;
	}
	kernel_printf("0x%X  %s\n", systemID, type->name);
	return error;
}