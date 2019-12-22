#ifndef _FS_OPEN_H_
#define _FS_OPEN_H_

#include <ouros/fs/fs.h>

bool qstr_equals(const qstr qstr, const char* str);
bool qstr_endwith(const char* filename, const qstr str);

int path_lookup(const char *filename, int flags, nameidata *nd);
int link_path_walk(const char *filename, nameidata *nd);
int do_lookup(nameidata *nd);
dentry* real_lookup(const nameidata *nd);
int follow_dotdot(nameidata *nd);

#endif