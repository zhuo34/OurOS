#ifndef _FS_H_
#define _FS_H_

#include <ouros/fs/fs.h>

#define 	MAX_PAR_COUNT 		4
#define 	SECTOR_SIZE 		512

// 主引导记录(MBR) Master Boot Record

// 硬盘分区表(DPT) Disk Partition Table
// 注意实际数据是小端的
typedef struct _DPT
{
	Byte 		dpt_indicator;						// 引导指示符(Boot Indicator)，指明该分区是否为活动分区, 0x80为活动，0x00为非活动
	Byte 		dpt_starting_H;						// 开始磁头(Head, 4bits)
	Word 		dpt_starting_S: 6;					// 开始扇区(Sector, 6bits)
	Word 		dpt_starting_C: 10;					// 开始柱面(Cylinder, 10bits)
	Byte 		dpt_systemID; 						// 系统ID，定义分区类型
	Byte 		dpt_ending_H;						// 结束磁头(Head, 4bits)
	Word 		dpt_ending_S:	6; 					// 结束扇区(Sector, 6bits)
	Word 		dpt_ending_C:	10;					// 结束柱面(Cylinder, 10bits)
	DWord 		dpt_base_addr;						// 该分区的起始LBA地址（相对于磁盘的开始），单位为扇区
	DWord 		dpt_sector_count;					// 该分区中的扇区总数
}DPT;

#define SYSTEM_ID_FAT32(x) 		((x) == 0x0B || (x) == 0x0C || (x) == 0x1B || (x) == 0x1C)
#define SYSTEM_ID_NTFS(x) 		((x) == 0x07)
#define SYSTEM_ID_EXT2(x) 		((x) == 0x83)

// 读取SD卡MBR信息
int read_MBR();
// 读取非对齐位置的数据
DWord read_unaligned(void *addr, uint size);
// 根据systemID获得对应的文件系统
file_system_type* get_fs_type(Byte systemID);

#endif
