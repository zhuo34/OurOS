#include "fat.h"

static file_system_type fs_type_fat32 = {
	.name 		= 		"fat32"
};

file_system_type* get_fs_type_fat32()
{
	return &fs_type_fat32;
}