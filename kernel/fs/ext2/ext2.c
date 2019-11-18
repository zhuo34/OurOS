#include "ext2.h"

static file_system_type fs_type_ext2 = {
	.name 		= 		"ext2"
};

file_system_type* get_fs_type_ext2()
{
	return &fs_type_ext2;
}