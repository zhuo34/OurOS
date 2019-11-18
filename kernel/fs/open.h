#ifndef _FS_OPEN_H_
#define _FS_OPEN_H_

#include <ouros/fs/fs.h>

bool qstr_equals(const qstr qstr, const char* str);
bool qstr_endwith(const char* filename, const qstr str);

#endif