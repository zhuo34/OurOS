#ifndef _FS_CMD_H_
#define _FS_CMD_H_

#include <linux/list.h>

#define PWD_BUF_SIZE			500
#define DEC_AND_PUT(str, c) 	( (*--(str)) = (c) )

// 命令
void pwd();
void ls(const char *dirname);
void cd(const char *dirname);
void cat(const char *filename);
void touch(const char *filename);
void mkdir(const char *filename);
void rm(const char *filename);
void mv(const char *src, const char *dst);
void app(const char *filename, const char *str);

// 输出命令行提示符
void print_prompt();

// 用于排序的比较函数
int sort_cmp(const struct list_head *a, const struct list_head *b);
// 输出错误码信息
void print_error_info(int code, const char *path);

#endif