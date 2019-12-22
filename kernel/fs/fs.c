#include "fs.h"

#include <driver/sd.h>

#include <ouros/log.h>
#include <ouros/error.h>
#include <ouros/assert.h>

#include <ouros/fs/fat.h>
#include <ouros/fs/ext2.h>
#include <ouros/fs/fscache.h>

#include "../usr/fs_cmd.h"

// temp
#define __max_buf  (1<<16)
static Byte malloc_buffer[__max_buf];
static int malloc_index = 0;

void* kmalloc_fake(uint size) {
	Byte* ret = malloc_buffer + malloc_index;
	// kernel_printf("before kmalloc: %d\n", malloc_index);
	malloc_index += size;
	malloc_index = (malloc_index + 3) & ~(0x3);
	// kernel_printf("after kmalloc: %d\n", malloc_index);
	if(malloc_index > __max_buf)
		ret = nullptr;
	// kernel_printf("malloc addr: 0x%X\n", ret);
	return ret; 
}
void  kfree_fake(void* p) {
	// kernel_printf("now stack index: %d\n", malloc_index);
}

// 超级块链表
static list_head *sb_list;

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
	} else {
		log(LOG_OK, "init_fscache");
	}

	// 读取SD卡分区表并初始化各分区的文件系统
	error = read_MBR();
	if(IS_ERR_VALUE(error)) {
		log(LOG_FAIL, "read_MBR, error code %d", error);
		return;
	} else {
		log(LOG_OK, "read_MBR");
	}

	// 初始化根目录项
	super_block *sb = container_of(sb_list->next, super_block, s_listnode);
	root_dentry = sb->s_root;
	d_inc_count(root_dentry);

	// 预读取根目录并创建对应的文件夹
	pwd_dentry = root_dentry;
	mkdir("/home");
	mkdir("/mnt");

	// 挂载其他分区的文件系统
	char name[] = "/mnt/fs0";
	const int index = 7;
	list_head *p;
	list_for_each(p, sb_list) {
		super_block *cur_sb = container_of(p, super_block, s_listnode);
		if(cur_sb == sb)
			continue;

		name[index] ++;
		file *fp = fs_open(name, F_MODE_READ_WRITE | F_MODE_DIR_MASK);
		if(IS_ERR_PTR(fp)) {
			log(LOG_FAIL, "mount to %s", name);
			return;
		}
		dentry *entry = fp->f_dentry;
		entry->d_mounted = true;
		cur_sb->s_mnt.mnt_parent = &sb->s_mnt;
		cur_sb->s_mnt.mnt_mntpoint = entry;

		fs_close(fp);

	}

	// 进入主目录
    cd("");

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

	// 从SD卡读取MBR数据
	if (!sd_read_block(buf_MBR, 0, 1)) {
		error = -ERROR_READ_SDCARD;
		goto exit;
	}

	// 初始化超级块链表
	sb_list = (list_head*)kmalloc_fake(sizeof(list_head));
	if(IS_ERR_PTR(sb_list)) {
		error = PTR_ERR(sb_list);
		goto exit;
	}
	INIT_LIST_HEAD(sb_list);
	
	// 解析MBR数据
	// 主引导记录占446字节，之后的数据为硬盘分区表(DPT) Disk Partition Table
	DPT *curDPT = (DPT*)(buf_MBR + 446);
	for(int i = 0; i < MAX_PAR_COUNT; i ++, curDPT ++) {
		// 获得当前分区的起始地址
		DWord base_addr = read_data(&curDPT->dpt_base_addr, sizeof(curDPT->dpt_base_addr));
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
		if(IS_ERR_PTR(sb)) {
			error = PTR_ERR(sb);
			goto exit;
		}
		list_add_tail(&sb->s_listnode, sb_list);
	}

exit:
	kfree_fake(buf_MBR);
	return error;
}

DWord read_data(void *addr, uint size)
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
	}

	// kernel_printf("0x%X  %s\n", systemID, type? type->name: "unknown");
	return type;
}

list_head* get_sb_list_entry()
{
	return sb_list;
}

dentry* get_root_dentry()
{
	return root_dentry;
}

dentry* get_pwd_dentry()
{
	return pwd_dentry;
}

void set_pwd_dentry(dentry* entry)
{
	pwd_dentry = entry;
}

void d_inc_count(dentry* entry) {
	entry->d_count ++;
	list_move(&entry->d_LRU, &get_fscache(D_CACHE)->LRU);
}

void d_dec_count(dentry* entry) {
	entry->d_count --;
	list_move(&entry->d_LRU, &get_fscache(D_CACHE)->LRU);
}

void i_inc_count(inode* node) {
	node->i_count ++;
	list_move(&node->i_LRU, &get_fscache(I_CACHE)->LRU);
}

void i_dec_count(inode* node) {
	node->i_count --;
	list_move(&node->i_LRU, &get_fscache(I_CACHE)->LRU);
}
