#ifndef SHELL_H
#define SHELL_H

// 单元测试时只包含test.h
#include "our_string.h"
#include "terminal.h"
#include <driver/ps2.h>
#include <driver/sd.h>
#include <driver/vga.h>
#include <os/bootmm.h>
#include <os/buddy.h>
#include <os/slab.h>
#include <os/time.h>
#include <os/utils.h>

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
typedef struct job* jobNode;
struct job
{
    pid_t pid;
    char name[MAX_ARGUMENT_LENGTH];
    jobStatus status;
    jobNode lastJob;
    jobNode nextJob;
};

void osh();
void parse_cmd(char* cmd);
void exec_cmd_pre(struct command* cmd);
void exec_cmd(struct command* cmd);

#endif // PS_H
