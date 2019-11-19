#include "fat.h"

#include <driver/sd.h>

#include <ouros/error.h>

static file_system_type fs_type_fat32 = {
	.name 		= 		"fat32",
	.get_sb 	= 		fat_get_sb
};

static super_block_op sb_op_fat32 = {

};

file_system_type* get_fs_type_fat32()
{
	return &fs_type_fat32;
}

super_block* fat_get_sb(DWord base)
{
	super_block* ret = nullptr;
	int error = NO_ERROR;

	// 申请缓存区读取扇区数据
	Byte *buf = (Byte*)kmalloc_fake(SECTOR_SIZE);
	if (buf == nullptr) {
		ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	// 从SD卡读取MBR数据
	if (!sd_read_block(buf, base, 1)) {
		ret = ERR_PTR(-ERROR_READ_SDCARD);
		goto exit;
	}
	// 读取DBR头部信息
	DBR_HEAD *dbr_head = (DBR_HEAD*)buf;

	// 构建超级块super_block
	ret = (super_block*)kmalloc_fake(sizeof(super_block));
	if (ret == nullptr) {
		ret = ERR_PTR(-ERROR_NO_MEMORY);
		goto exit;
	}
	ret->s_op 			= &sb_op_fat32;
	ret->s_type 		= &fs_type_fat32;
	ret->s_blocksize 	= dbr_head->dbr_sec_per_clu << SECTOR_SHIFT;
	ret->s_root 		= ;
	INIT_LIST_HEAD(&ret->s_listnode);

	// 初始化根目录inode

	// 初始化根目录dentry

	// 初始化根目录vfsmount
	vfsmount *mnt = &ret->s_mnt;
	mnt->mnt_parent = nullptr;
	mnt->mnt_mntpoint = nullptr;
	mnt->mnt_sb = ret;

exit:
	kfree_fake(buf);
	return ret;
}