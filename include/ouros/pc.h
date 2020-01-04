#ifndef OS_PC_H
#define OS_PC_H

#include <arch.h>
#include <intr.h>
#include <ouros/shm.h>
#include <ouros/type.h>
#include <ouros/utils.h>
#include <tlb.h>

// 内存接口
#include <ouros/mm.h>
#include <ouros/vm.h>

// 文件系统接口
#include <ouros/fs/fs.h>

#define KERNEL_STACK_SIZE 4096
#define TASK_NAME_MAX 16
#define PID_STATUS_SIZE 32
#define SEM_MAX 16

typedef unsigned int pid_t;

enum task_state { PREPARING, RUNNING, INTERRUPTIBLE, STOPPED, ZOMBIE };
enum task_mode { USER, KERNEL };
enum signal { SIGSTOP = 1, SIGCHILD = 2, SIGWAIT = 4, SIGRESERVE = 8 };

struct regs_context
{
    u32 epc;
    u32 at;
    u32 v0, v1;
    u32 a0, a1, a2, a3;
    u32 t0, t1, t2, t3, t4, t5, t6, t7;
    u32 s0, s1, s2, s3, s4, s5, s6, s7;
    u32 t8, t9;
    u32 hi, lo;
    u32 gp, sp, fp, ra;
};

struct task_list_node
{
    struct task_list_node* last;
    struct task_list_node* next;
};

typedef void(*sig_handler)();

// 进程描述符
struct task_struct
{
    // 进程号和父进程号
    pid_t pid;
    pid_t parent;
    // 进程asid号
    // unsigned int asid;
    //进程状态
    enum task_state status;
    //进程名
    unsigned char name[TASK_NAME_MAX];
    // 上下文信息
    struct regs_context context;
    // 进程的地址空间描述符
    struct mm_struct* mm;
    // 进程打开的文件
    file* fs;
    // 链表节点段
    struct task_list_node node;
    struct task_list_node node_shedule;
    // 信号位状态
    unsigned int signals;
    // 子进程退出时的信号处理
    sig_handler handler;
};

extern struct task_struct* current;

// 进程描述符存储于内核栈的底部
union task_union
{
    struct task_struct task;
    unsigned char kernel_stack[KERNEL_STACK_SIZE];
};

// 信号量的结构
struct semaphore
{
    int key;
    int value;
};

// 暴漏给外部的函数
void init_pc();
pid_t task_create(char* name, void(*entry)(unsigned int argc, void* argv), unsigned int argc, void* argv, enum task_mode mode);
void wakeup();
void sigHandler(enum signal sig, sig_handler handler);
void kill(pid_t pid, enum signal sig);
void waitpid(pid_t pid);
void loadUserProgram(char* fileName);
struct semaphore* getSem(int key, int value);
void P(struct semaphore* sem);
void V(struct semaphore* sem);

#endif  // OS_PC_H