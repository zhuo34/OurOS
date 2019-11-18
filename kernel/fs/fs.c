#include "fs.h"

#include <driver/sd.h>

#include <ouros/log.h>
#include <ouros/error.h>
#include <ouros/assert.h>

#include <ouros/fs/fat.h>
#include <ouros/fs/ext2.h>
#include <ouros/fs/ntfs.h>
#include <ouros/fs/fscache.h>

// temp
#define __max_buf  (1<<16)
static Byte malloc_buffer[__max_buf];
static int malloc_index = 0;

void* kmalloc_fake(uint size) {
	Byte* ret = malloc_buffer + malloc_index;
	// kernel_printf("before kmalloc: %d\n", malloc_index);
	malloc_index += size;
	// kernel_printf("after kmalloc: %d\n", malloc_index);
	if(malloc_index > __max_buf)
		ret = nullptr;
	return ret; 
}
void  kfree_fake(void* p) {

}

// 超级块链表
list sb_list = {
	.head 	= 	nullptr,
	.size 	= 	0
};

static dentry *root_dentry;
static dentry *pwd_dentry;

void init_fs()
{
	malloc_index = 0;
	int error = NO_ERROR;

	// 初始化文件系统缓存区
	error = init_fscache();
	if(IS_ERR_VALUE(error)) {
		log(LOG_FAIL, "init_fscache, error code %d", error);
		return;
	}
	log(LOG_OK, "init_fscache");

	// 读取SD卡分区表
	error = read_MBR();
	if(IS_ERR_VALUE(error)) {
		log(LOG_FAIL, "read_MBR, error code %d", error);
		return;
	}
	log(LOG_OK, "read_MBR");

	// 初始化各分区文件系统
	// for(int i=0; i<sdCard_MBR->par_count; i++) {
	// 	sdCard_MBR->par_fs_system[i]->getSuperBlock;
	// 	superblock->init(baseaddr)
	// }
}

int read_MBR()
{
	int error = NO_ERROR;
	
	// 申请buffer读取MBR数据
	Byte* buf_MBR = (Byte*)kmalloc_fake(SECTOR_SIZE);
	if (buf_MBR == nullptr) {
		error = -ERROR_NO_MEMORY;
		goto exit;
	}
	kernel_memset_uint(buf_MBR, 0, SECTOR_SIZE);

	// 从SD卡读取MBR数据
	if (!sd_read_block(buf_MBR, 0, 1)) {
		error = -ERROR_READ_MBR;
		goto exit;
	}

	// 解析MBR数据
	// 主引导记录占446字节，之后的数据为硬盘分区表(DPT) Disk Partition Table
	DPT *curDPT = (DPT*)(buf_MBR + 446);
	for(int i = 0; i < MAX_PAR_COUNT; i ++, curDPT ++) {
		// 获得当前分区的起始地址
		DWord base_addr = read_unaligned(&curDPT->dpt_base_addr, sizeof(curDPT->dpt_base_addr));
		if(!base_addr) 
			continue;
		// kernel_printf("dpt_base_addr: %d\n", base_addr);

		// 根据systemID获得当前分区的文件系统类型
		file_system_type *type = get_fs_type(curDPT->dpt_systemID);
		if(!type) {
			error = -ERROR_UNKNOWN_FS;
			goto exit;
		}
		// 根据文件系统类型创建超级块
		super_block *sb = type->get_sb(base_addr);
		if(sb == nullptr) {
			error = -ERROR_NO_MEMORY;
			goto exit;
		}
		append_list_node(&sb_list, &sb->s_listnode);

	}

exit:
	kfree_fake(buf_MBR);
	return error;
}

DWord read_unaligned(void *addr, uint size)
{
	kernel_assert(size > 0 && size <= 4, "read_unaligned");

	DWord ret = 0;
	kernel_memcpy(&ret, addr, size);
	return ret;
}

file_system_type* get_fs_type(Byte systemID)
{
	file_system_type *type = nullptr;
	if(SYSTEM_ID_FAT32(systemID)) {
		type = get_fs_type_fat32();
	} else if(SYSTEM_ID_EXT2(systemID)) {
		type = get_fs_type_ext2();
	} else if(SYSTEM_ID_NTFS(systemID)) {
		type = get_fs_type_ntfs();
	}

	// kernel_printf("0x%X  %s\n", systemID, type? type->name: "unknown");
	return type;
}

dentry* get_root_dentry()
{
	return root_dentry;
}

dentry* get_pwd_dentry()
{
	return pwd_dentry;
}