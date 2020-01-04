#include "shell.h"
#include "fs_cmd.h"

char ps_buffer[64];

// ourShell的主函数
void osh()
{
    // 初始化
    char cmd[MAX_COMMAND_LENGTH];
    kernel_clear_screen();
    // 原ZJUNIX怎么还PowerShell的
    kernel_puts("OurShell Starts!\n\n");

    print_prompt();
    while (1)
    {
        // 从标准输入读入，传给解析器解析
        if (read_line(cmd, MAX_COMMAND_LENGTH))
        {
            parse_cmd(cmd);
        }
        print_prompt();
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
    struct command* thisCmd = (struct command*)kmalloc(sizeof(struct command));
    kernel_memset(thisCmd->cmdName, 0, MAX_ARGUMENT_LENGTH);
    // 参数链表
    thisCmd->argList = (struct argumentNode*)kmalloc(sizeof(struct argumentNode));
    struct argumentNode* thisArg = thisCmd->argList;
    thisArg->nextArg = (void*)0;
    
    int cmdIndex = 0, argIndex = 0;

    int len = kernel_strlen(cmd) + 1;
    // 扫描一遍字符串
    // 看不懂两个月前写的代码系列
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
    exec_cmd(thisCmd);
}

// 子进程执行命令
void exec_cmd(struct command* cmd)
{
    if(kernel_strcmp(cmd->cmdName, "pwd") == 0)
    {
        pwd();
    } 
    else if (kernel_strcmp(cmd->cmdName, "ls") == 0)
    {
        char* name = cmd->argList->nextArg ? cmd->argList->nextArg->argName : nullptr;
        ls(name);
    } 
    else if (kernel_strcmp(cmd->cmdName, "cd") == 0)
    {
        char* name = cmd->argList->nextArg ? cmd->argList->nextArg->argName : nullptr;
        cd(name);
    } 
    else if (kernel_strcmp(cmd->cmdName, "cat") == 0)
    {
        if(!cmd->argList->nextArg)
        {
            kernel_printf("%s: not enough arguments. \n", cmd->cmdName);
			return;
		}
        cat(cmd->argList->nextArg->argName);
    }
    else
    {
        kernel_printf("%s: unknown command. \n", cmd->cmdName);
    }
}

struct argumentNode* newArg(struct argumentNode* arg)
{
    arg->nextArg = (struct argumentNode*)kmalloc(sizeof(struct argumentNode));
    struct argumentNode* thisArg = arg->nextArg;
    kernel_memset(thisArg->argName, 0, MAX_ARGUMENT_LENGTH);
    thisArg->nextArg = (void*)0;
    return thisArg;
}

// 从标准输入获取一行
bool read_line(char* str, int length)
{
    int c;
    int index = 0;
    while (1)
    {
        c = kernel_getchar();
        // 回车，结束输入
        if ('\n' == c)
        {
            // 确保字符串结尾是'\0'
            // 在自己的电脑上测试不需要结尾的\n
            // 但是在板子上测试必须加上\n
            // 否则最后一个字符必须是空格，正常字符读不到
            str[index] = c;
            str[index + 1] = 0;
            return true;
        }
        // 回退，删除一个字符，将其从数组中删除，也从屏幕上消失
        else if ('\b' == c)
        {
            if (index)
            {
                // 原ZJUNIX代码只有index--，这时按下回车不就出bug了
                str[index--] = '\0';
                // 删除字符，重置光标
                kernel_putchar_at(' ', cursor.row, cursor.col - 1);
                cursor.col--;
                kernel_set_cursor();
            }
        }
        // 输入了EOF，现在应该还遇不到
        else if (-1 == c)
        {
            return false;
        }
        // 输入一般字符，存在字符串中并显示出来
        else
        {
            // length要减1，结尾给'\0'留个位置
            if (index < length - 1)
            {
                str[index++] = c;
                kernel_putchar(c);
            }
        }
    }
}