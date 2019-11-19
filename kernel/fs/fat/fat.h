#ifndef FAT_H
#define FAT_H

#include <ouros/fs/fat.h>

// 分区引导扇区(DBR) Dos Boot Record
// 总共512字节，这里的结构体取前面48字节的有效信息
#pragma pack(push)
#pragma pack(1)

typedef struct _DBR_HEAD
{
	Byte 		dbr_jump1; 				// 前两个字段表示跳转指令，共占3字节
	Word		dbr_jump2;				// 见上面说明
	QWord 		dbr_version; 			// 文件系统标志和版本号，占8字节
	Word 		dbr_byte_per_sec; 		// 每扇区字节数，占2字节，值应为512
	Byte 		dbr_sec_per_clu; 		// 每簇扇区数，占1字节，理论上值应为8
	Word 		dbr_preserve_num; 		// 保留扇区数，占2字节，理论上值应为38
	Byte 		dbr_fat_num; 			// fat表个数，占1字节，值为2
	Word 		dbr_preserved1; 		// fat32中该字段为0，占2字节
	Word 		dbr_preserved2; 		// fat32中该字段为0，占2字节
	Byte 		dbr_medium; 			// 存储介质类型，占1字节
	Word 		dbr_preserved3; 		// fat32中该字段为0，占2字节
	Word 		dbr_sec_per_track; 		// 每磁道扇区数，占2字节
	Word 		dbr_maghead_num; 		// 磁头数，占2字节
	DWord 		dbr_hidden; 			// EBR分区之前所隐藏的扇区数，占4字节
	DWord 		dbr_total_sec_num;		// 总扇区数，占4字节
	DWord 		dbr_fat_secsize; 		// fat表占用的扇区数，占4字节
	Word 		dbr_symbol_fat32; 		// fat32标记域，占2字节
	Word 		dbr_version_fat32; 		// fat32版本号，占2字节
	DWord 		dbr_root_clu; 			// 根目录所在的起始簇号，占4字节
}DBR_HEAD;

#pragma pack(pop)

super_block* fat_get_sb(DWord base);

#endif  // FAT_H
