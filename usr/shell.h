#ifndef SHELL_H
#define SHELL_H

#include "terminal.h"
#include <driver/ps2.h>
#include <driver/sd.h>
#include <driver/vga.h>
#include <ouros/bootmm.h>
#include <ouros/buddy.h>
#include <ouros/slab.h>
#include <ouros/time.h>
#include <ouros/utils.h>

#define MAX_COMMAND_LENGTH 64
#define MAX_ARGUMENT_LENGTH 8

struct argumentNode
{
    char argName[MAX_ARGUMENT_LENGTH];
    struct argumentNode* nextArg;
};

struct command
{
    char cmdName[MAX_ARGUMENT_LENGTH];
    struct argumentNode* argList;
};

struct argumentNode* newArg(struct argumentNode* arg);

// 进程表，放进共享内存里
// typedef struct job* jobNode;
// struct job
// {
//     pid_t pid;
//     char name[MAX_ARGUMENT_LENGTH];
//     jobStatus status;
//     jobNode lastJob;
//     jobNode nextJob;
// };

void osh();
void parse_cmd(char* cmd);
void exec_cmd_pre(struct command* cmd);
void exec_cmd(struct command* cmd);

// 从标准输入获取一行
bool read_line(char* str, int length);

#endif // PS_H
