#include "ntfs.h"

static file_system_type fs_type_ntfs = {
	.name 		= 		"ntfs"
};


file_system_type* get_fs_type_ntfs()
{
	return &fs_type_ntfs;
}