// 用于shell的单元测试

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int our_gets(char* str, int maxlength);
void kernel_putstring(char* str, int fg, int bg);
void kernel_strcpy(char* dst, const char* src);

int our_fork();
