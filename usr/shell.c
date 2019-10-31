#include "shell.h"

char ps_buffer[64];

// ourShell的主函数
void osh()
{
    // 初始化
    char cmd[MAX_COMMAND_LENGTH];
    kernel_clear_screen();
    // 原ZJUNIX怎么还PowerShell的，这Windows遗毒是有多深
    kernel_puts("OurShell Starts!\n\n");

    kernel_puts("osh>");
    while (1)
    {
        // 从标准输入读入，传给解析器解析
        if (read_line(cmd, MAX_COMMAND_LENGTH))
        {
            parse_cmd(cmd);
        }
        kernel_puts("osh>");
    }
}

// 命令格式统一只许argument，不许option
// ls help -> YES
// ls -h -> NO
void parse_cmd(char* cmd)
{
    char thisArgument[MAX_ARGUMENT_LENGTH];
    // 有限状态机，0为读命令，1为读参数，做管道和重定向时加状态
    // 状态：读入命令名
    int status = 0;

    // 目前为单个命令，如果支持多条命令，也用链表
    struct command* thisCmd = (struct command*)kernel_malloc(sizeof(struct command));
    // 参数链表
    thisCmd->argList = (struct argumentNode*)kernel_malloc(sizeof(struct argumentNode));
    struct argumentNode* thisArg = thisCmd->argList;
    thisArg->nextArg = (void*)0;
    

    int cmdIndex = 0, argIndex = 0;
    // 扫描一遍字符串
    while (cmd[cmdIndex] && cmdIndex < MAX_COMMAND_LENGTH)
    {
        // 正常字符
        if (' ' != cmd[cmdIndex] && '\0' != cmd[cmdIndex] && '\n' != cmd[cmdIndex])
        {
            if (argIndex < MAX_ARGUMENT_LENGTH - 1)
            {
                thisArgument[argIndex++] = cmd[cmdIndex++];
                continue;
            }
            else
            {
                kernel_printf_error("argument too long!\n");
                break;
            }
        }
        // 遇到空格或结束
        else
        {
            // 一个参数读入完毕，要放进链表，准备读下一个或结束
            if (argIndex)
            {
                thisArgument[argIndex] = '\0';
                // 这是命令名
                if (0 == status)
                {
                    kernel_strcpy(thisCmd->cmdName, thisArgument);
                    status = 1;
                }
                // 这是参数
                else if (1 == status)
                {
                    thisArg = newArg(thisArg);
                    kernel_strcpy(thisArg->argName, thisArgument);
                }
                // 结束
                if ('\0' == cmd[cmdIndex] || '\n' == cmd[cmdIndex])
                {
                    break;
                }
                // 读下一个
                else
                {
                    argIndex = 0;
                    continue;
                }
            }
            // 连续的空格
            else
            {
                // 读完了
                if ('\0' == cmd[cmdIndex] || '\n' == cmd[cmdIndex])
                {
                    break;
                }
                // 连续空格
                else
                {
                    cmdIndex++;
                    continue;
                }
            }
        }
    }
    exec_cmd_pre(thisCmd);
}

void exec_cmd_pre(struct command* cmd)
{
    int pid = kernel_fork();
    switch(pid)
    {
        case -1:
            kernel_printf_error("fork fail\n\n");
            kernel_exit(1);
        // 暂时不考虑后台命令
        // 子进程
        case 0:
        {
            // 阻塞，在进程表找到自己的位置
            // 等主进程放行
        }
        // 主进程
        default:
        {
            // 把子进程的信息写入进程表，然后放行
            // 阻塞，等待SIGCHLD信号
            // 把子进程从进程表里删掉
        }
    }
}

// 子进程执行命令
void exec_cmd(struct command* cmd)
{
    // 内建命令所用函数的接收参数统一为struct command*
    kernel_printf_error("command is running\n\n");
}

struct argumentNode* newArg(struct argumentNode* arg)
{
    arg->nextArg = (struct argumentNode*)kernel_malloc(sizeof(struct argumentNode));
    struct argumentNode* thisArg = arg->nextArg;
    thisArg->nextArg = (void*)0;
    return thisArg;
}

// 原ZJUNIX里有的这里没有的：一堆内建命令，每个实现为一个函数
// 还要添加：进程表初始化、新建、删除、查找的函数

int kernel_fork() { return 1; }
void kernel_exit(int code) { }
void* kernel_malloc(uint size) { return nullptr; }