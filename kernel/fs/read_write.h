#ifndef _FS_READ_WRITE_H_
#define _FS_READ_WRITE_H_

#include <ouros/fs/fs.h>
#include <ouros/utils.h>

int fs_printf_argList(file *fp, const char* format, va_list argList);
int fs_puthex(file *fp, uint hex, bool isUpper);

int file_expand_to(file *fp, int size);

#endif