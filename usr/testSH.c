// 用于shell的单元测试

#include "test.h"
#include "shell.h"

int main()
{
    osh();
}

void kernel_strcpy(char* dst, const char* src)
{
    strcpy(dst, src);
}

void kernel_putstring(char* str, int fg, int bg)
{
    printf("%s", str);
}

int our_gets(char* str, int maxlength)
{
    fgets(str, maxlength, stdin);
    return 1;
}

int our_fork()
{
    //测试父进程，测试子进程直接返回0
    static int i = 100;
    return i++;
}