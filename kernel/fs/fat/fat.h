#ifndef FAT_H
#define FAT_H

#include <ouros/fs/fat.h>

// 分区引导扇区(DBR) Dos Boot Record
// 总共512字节，这里的结构体取前面48字节的有效信息
#pragma pack(push)
#pragma pack(1)

typedef struct _dbr_head
{
	Byte 		dbr_jump[3]; 			// 跳转指令，占3字节
	QWord 		dbr_version; 			// 文件系统标志和版本号，占8字节
	Word 		dbr_byte_per_sec; 		// 每扇区字节数，占2字节，值应为512
	Byte 		dbr_sec_per_clu; 		// 每簇扇区数，占1字节，理论上值应为8
	Word 		dbr_reserved_num; 		// 保留扇区数，占2字节，理论上值应为38
	Byte 		dbr_fat_num; 			// fat表个数，占1字节，值为2
	Word 		dbr_reserved1; 			// fat32中该字段为0，占2字节
	Word 		dbr_reserved2; 			// fat32中该字段为0，占2字节
	Byte 		dbr_medium; 			// 存储介质类型，占1字节
	Word 		dbr_reserved3; 			// fat32中该字段为0，占2字节
	Word 		dbr_sec_per_track; 		// 每磁道扇区数，占2字节
	Word 		dbr_maghead_num; 		// 磁头数，占2字节
	DWord 		dbr_hidden; 			// EBR分区之前所隐藏的扇区数，占4字节
	DWord 		dbr_total_sec_num;		// 总扇区数，占4字节
	DWord 		dbr_fat_secsize; 		// fat表占用的扇区数，占4字节
	Word 		dbr_symbol_fat32; 		// fat32标记域，占2字节
	Word 		dbr_version_fat32; 		// fat32版本号，占2字节
	DWord 		dbr_root_clu; 			// 根目录所在的起始簇号，占4字节
}dbr_head;


#define SFN_MARK_DELETED 		0xE5
#define SFN_MARK_UNUSED 		0x00

#define SFN_ATTR_WRITE 			0x00
#define SFN_ATTR_READONLY		0x01
#define SFN_ATTR_HIDDEN 		0x02
#define SFN_ATTR_SYSTEM 		0x04
#define SFN_ATTR_LABEL 			0x08
#define SFN_ATTR_DIR 			0x10
#define SFN_ATTR_ARCHIVE 		0x20
#define SFN_ATTR_LFN			0x0F

// 短文件名(SFN) Short File Name
// 长文件名(LFN) Long  File Name，本文件系统不处理
typedef struct _sfn_entry
{
	union {
		Byte sfn_mark; 					// 标志位
		struct filename_8dot3 {
			Byte 	name[8]; 			// 主文件名
			Byte 	ext[3]; 			// 扩展名
		} sfn_filename; 				// 文件名，8+3格式
	};
	
    Byte 		sfn_attr; 				// 文件属性
    Byte 		sfn_reserved; 			// 保留字段
    Byte 		sfn_create_time_10ms; 	// 创建时间的10毫秒位
    Word 		sfn_create_time; 		// 创建时间的时、分、秒位，0-4:秒，单位为2s；5-10:分；11-15:时 
    Word 		sfn_create_date;  		// 创建日期，0-4:日；5-8:月；9-15:年，相对于1980年计数，因此需加上1980
    Word 		sfn_access_date; 		// 最近访问日期
    Word 		sfn_1stclu_high; 		// 起始簇号的高16位
    Word 		sfn_modify_time; 		// 最近修改时间
    Word 		sfn_modify_date; 		// 最近修改日期
    Word 		sfn_1stclu_low; 		// 起始簇号的低16位
    DWord 		sfn_file_size; 			// 文件字节大小
}sfn_entry;

#pragma pack(pop)

// 读取FSINFO扇区中记录的下一可用簇号值
#define READ_FSINFO_NEXT_CLU(buffer) 			(*(DWord*)( (Byte*)(buffer) + 492) )
#define WRITE_FSINFO_NEXT_CLU(buffer, value) 	(*(DWord*)( (Byte*)(buffer) + 492) = (value) )

struct _fs_info_fat32
{
	DWord 		info_fsinfo_sec; 		// FSINFO所在扇区
	DWord 		info_fat_sec;			// fat表起始扇区
	DWord 		info_clu_base; 			// 根簇起始扇区
	DWord 		info_next_clu; 			// 寻找下一个可用簇号的起始位置
};

// 注意FAT中是没有0号和1号簇的，fat表后第一个簇即为2号簇！
#define CLU_TO_SEC(base_sec, blk_num, blk_bits) \
	((base_sec) + ( ( max(0, (blk_num) - 2) << (blk_bits)) >> SECTOR_SHIFT))
#define SEC_TO_CLU(base_sec, sec_num, blk_bits) \
	( ( ( ((sec_num) - (base_sec)) << SECTOR_SHIFT) >> (blk_bits)) + 2)

// 读取FAT表项的值
#define FAT_ITEM_FREE 				0x00000000
#define FAT_ITEM_END 				0x0FFFFFFF
DWord read_fat(DWord fat_base_sec, DWord clu_num);
int write_fat(DWord fat_base_sec, DWord clu_num, DWord value);
DWord find_free_clu(DWord fat_base_sec, DWord start_idx);
int update_next_clu(super_block *sb, DWord value);

// 文件名转换
#define DUMMY_CHAR_8DOT3 			(' ')
void convert_normal_to_8dot3(const qstr* src, struct filename_8dot3* dst);
void convert_8dot3_to_normal(const struct filename_8dot3* src, qstr* dst);

dentry* read_dentry_from_sfn(sfn_entry* sfn_item, dentry *parent_entry, qstr name);

// fs_type op
super_block* get_sb_fat32(DWord base_sec);

// super_block_op
inode* 	alloc_inode_fat32(super_block* sb, uint base_blk_addr, uint size, bool isdir);
dentry* alloc_dentry_fat32(super_block* sb, inode* node, dentry* parent, const qstr name);
dentry* create_dentry_fat32(super_block* sb, dentry* parent, const qstr name, bool isdir);

// inode_op
dentry* lookup_fat32(dentry* parent_entry, const qstr name);
int expand_fat32(inode* node, int num);
int clear_fat32(inode* node, dentry* parent, const qstr name);
int delete_fat32(inode* node, dentry* parent, const qstr name);

// dentry_op
int compare_fat32(const qstr a, const qstr b);

// file op
dentry* list_dir_fat32(file* file);
int close_fat32(file* file);

// inline function
static inline char TO_UPPER_CASE(char c)
{
	if(c >= 'a' && c <= 'z')
		c -= 'a' - 'A';
	return c;
}

static inline char TO_LOWER_CASE(char c)
{
	if(c >= 'A' && c <= 'Z')
		c -= 'A' - 'a';
	return c;
}

#endif  // FAT_H
