#include <ouros/pc.h>
#include <arch.h>
#include <ouros/utils.h>

// debug
#include <driver/vga.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

pid_t task_create(char* name, void(*entry)(unsigned int argc, void* argv), unsigned int argc, void* argv, enum task_mode mode);
pid_t pid_alloc();
void pid_delete(pid_t pid);
void pc_schedule(unsigned int status, unsigned int cause, struct regs_context* pt_context);
struct task_struct* find_next_task();
struct task_struct* find_task(struct task_list_node* node);
struct task_struct* find_task_pid(struct task_list_node* head, pid_t pid);
void task_list_init(struct task_list_node* node);
void task_list_add(struct task_list_node* list, struct task_list_node* node);
void task_list_delete(struct task_list_node* node);
void list_print(struct task_list_node* node);
void list_print_reverse(struct task_list_node* node);
void idle();
void activateMemory(struct mm_struct* mm);
void copy_context(context* src, context* dst);
void wakeup();
void kill(pid_t pid, enum signal sig);
void sigHandler(enum signal sig, sig_handler handler);
void waitpid(pid_t pid);
void clearup();

// 当前进程
struct task_struct* current;

// 进程表
struct task_list_node task_list;
// 等待进程表
struct task_list_node task_wait_list;
// 结束进程表
struct task_list_node task_exit_list;
// 进程调度表
struct task_list_node task_schedule_list;

// 保存进程号是否已被分配，一共可以分配PID_STATUS_SIZE * 8个进程号
unsigned char pid_status[PID_STATUS_SIZE];

// 初始化进程模块
// 创建0号进程，初始化进程表
void init_pc()
{
    // 初始化各链表
    task_list_init(&task_list);
    task_list_init(&task_wait_list);
    task_list_init(&task_exit_list);
    task_list_init(&task_schedule_list);
    
    // 0号进程，进程描述符位于内核栈底
    struct task_struct* idle = (struct task_struct*)(kernel_sp - KERNEL_STACK_SIZE);
    idle->pid = 0;
    idle->parent = 0;
    idle->status = PREPARING;
    // idle->asid = 0;
    kernel_strcpy(idle->name, "idle");
    pid_status[0] = 1;
    idle->fs = (void*)0;
    idle->signals = 0;
    idle->mm = 0;

    // 将0号进程加入链表
    idle->status = RUNNING;
    current = idle;

    task_list_add(&task_list, &(idle->node));
    task_list_add(&task_schedule_list, &(idle->node_shedule));

    // 注册调度函数，即将由调度器一锤定音定于一尊
    register_interrupt_handler(7, pc_schedule);
    asm volatile(
        "li $v0, 1000000\n\t"
        "mtc0 $v0, $11\n\t"
        "mtc0 $zero, $9");
}

// 激活地址空间
void activateMemory(struct mm_struct* mm)
{
    // init_tlb();
    // kernel_printf("test6\n");
    // kernel_printf("mm: 0x%x\n", mm);
    int asid = 0;
    if (mm)
    {
        asid = mm->asid;
    }
    
    asm volatile(
        "move $t0, %0\n\t"
        "and $t0, $t0, 0xff\n\t"
        "mfc0 $t1, $10\n\t"
        "and $t1, $t1, 0xffffff00\n\t"
        "or $t0, $t0, $t1\n\t"
        "mtc0 $t0, $10\n\t"
        :
        :"r"(asid));
    // kernel_printf("test7\n");
    mm_current = mm;
    // kernel_printf("test8\n");
}

// 获取cp0的十号寄存器内容
int getASID()
{
    int x;
    asm volatile(
        "mfc0 %0, $10\n\t"
        :"=r"(x));
    kernel_printf("%d %d\n", x);
}

// 创建新进程，返回进程号
// 若返回(pid_t)-1，则为创建失败
pid_t task_create(char* name, void(*entry)(unsigned int argc, void* argv), unsigned int argc, void* argv, enum task_mode mode)
{
    // 为新进程分配pid号并创建内核栈
    pid_t pid = pid_alloc();
    if ((pid_t)-1 == pid)
    {
        return pid;
    }
    union task_union* task_new = (union task_union*)kmalloc(sizeof(union task_union));
    struct task_struct* task_ptr = (struct task_struct*)task_new;
    
    task_ptr->pid = pid;
    task_ptr->parent = current->pid;
    // task_ptr->asid = pid;
    task_ptr->status = PREPARING;
    kernel_strcpy(task_ptr->name, name);
    
    // 寄存器上下文信息清零
    kernel_memset(&(task_ptr->context), 0, sizeof(struct regs_context));
    // 程序计数器的值为程序首地址
    task_ptr->context.epc = (unsigned int)entry;
    // 内核栈顶
    task_ptr->context.sp = (unsigned int)task_new + KERNEL_STACK_SIZE;
    unsigned int gp;
    asm volatile("la %0, _gp\n\t" : "=r"(gp));
    task_ptr->context.gp = gp;
    // 传参
    task_ptr->context.a0 = argc;
    task_ptr->context.a1 = (unsigned int)argv;
    
    // 信号位清零
    task_ptr->signals = 0;
    // 为用户进程分配地址空间
    if (KERNEL == mode)
    {
        task_ptr->mm = (void*)0;
    }
    else
    {
        task_ptr->mm = create_mm_struct(task_ptr->pid);
    }
    // 文件系统
    task_ptr->fs = (void*)0;
    
    task_list_add(&task_list, &(task_ptr->node));
    task_list_add(&task_schedule_list, &(task_ptr->node_shedule));
    // list_print(&task_list);
    // 进入运行状态
    task_ptr->status = RUNNING;
    // list_print(&task_schedule_list);
    return pid;
}

// 用户进程的创建入口
pid_t task_create_(pid_t pid, char* name, void* epc,
                        enum task_mode mode, struct mm_struct* mm)
{
    if ((pid_t)-1 == pid)
    {
        return pid;
    }
    union task_union* task_new = (union task_union*)kmalloc(sizeof(union task_union));
    struct task_struct* task_ptr = (struct task_struct*)task_new;
    
    task_ptr->pid = pid;
    task_ptr->parent = current->pid;
    // task_ptr->asid = pid;
    task_ptr->status = PREPARING;
    kernel_strcpy(task_ptr->name, name);
    
    // 寄存器上下文信息清零
    kernel_memset(&(task_ptr->context), 0, sizeof(struct regs_context));
    // 程序计数器的值为程序首地址
    task_ptr->context.epc = (unsigned int)epc;
    // 内核栈顶
    task_ptr->context.sp = (unsigned int)task_new + KERNEL_STACK_SIZE;
    unsigned int gp;
    asm volatile("la %0, _gp\n\t" : "=r"(gp));
    task_ptr->context.gp = gp;
    
    // 信号位清零
    task_ptr->signals = 0;
   
    task_ptr->mm = mm;
    
    // 文件系统
    task_ptr->fs = (void*)0;
    
    task_list_add(&task_list, &(task_ptr->node));
    task_list_add(&task_schedule_list, &(task_ptr->node_shedule));
    // list_print(&task_list);
    // 进入运行状态
    task_ptr->status = RUNNING;
    // list_print(&task_schedule_list);
    return pid;
}

// 进程退出
void task_exit()
{
    // 修改进程状态
    current->status = ZOMBIE;

    // 释放进程地址空间
    if (current->mm)
    {
        // mm_delete(current->mm);
    }
    // 释放文件描述符
    if (current->fs)
    {
        // fs_delete(current->fs);
    }

    // 向父进程发送SIGCHILD信号
    wakeup();
    
    // 从调度表中移除当前进程
    task_list_delete(&(current->node_shedule));
    // 向退出进程表中加入当前进程
    task_list_add(&task_exit_list, &(current->node));
    // 释放当前进程的pid
    pid_delete(current->pid);
    
    // 控制权回到进程调度器
    struct task_struct* next = find_next_task();
    current = next;
    // switch_ex(&(current->context));
}

// 唤醒父进程，即发送SIGCHILD信号
void wakeup()
{
    kill(current->parent, SIGCHILD);
}

// 进程调度
void pc_schedule(unsigned int status, unsigned int cause, struct regs_context* pt_context)
{
    clearup();
    // kernel_printf("sche\n");
    if (current->status == RUNNING)
    {
        task_list_delete(&(current->node_shedule));
        task_list_add(&task_schedule_list, &(current->node_shedule));
    }
    // 优先级算法
    // struct task_struct* next;
    // find:
    // next = find_next_task();
    struct task_struct* next;
    // 先查看等待链表
    for (struct task_list_node* it = task_wait_list.next; it != &task_wait_list; it = it->next)
    {
        next = container_of(it, struct task_struct, node_shedule);
        if (next->signals)
        {
            // 收到进程终止信号
            if (next->signals & SIGSTOP)
            {
                // 从调度表中移除当前进程
                task_list_delete(&(next->node_shedule));
                // 向退出进程表中加入当前进程
                task_list_add(&task_exit_list, &(next->node_shedule));
                // 释放进程的pid
                pid_delete(next->pid);
                next->signals ^= SIGSTOP;
                goto find;
            }
            // 收到SIGCHILD信号，若进程处于等待状态，则恢复
            else if (next->signals & SIGCHILD)
            {
                if (next->status == INTERRUPTIBLE)
                {
                    // 从等待链表中移除当前进程
                    task_list_delete(&(next->node_shedule));
                    // 向调度表中加入当前进程
                    task_list_add(&task_schedule_list, &(next->node_shedule));
                    // 修改进程状态
                    next->status = RUNNING;
                    next->signals ^= SIGCHILD;
                }
                // 抢占执行
                goto finded;
            }
        }
    }

    find:
    {
    next = container_of(task_schedule_list.next, struct task_struct, node_shedule);
    if (next->signals)
    {
        // 收到进程终止信号
        if (next->signals & SIGSTOP)
        {
            // 从调度表中移除当前进程
            task_list_delete(&(next->node_shedule));
            // 向退出进程表中加入当前进程
            task_list_add(&task_exit_list, &(next->node_shedule));
            // 释放进程的pid
            pid_delete(next->pid);
            next->signals ^= SIGSTOP;
            goto find;
        }
        // 收到SIGWAIT信号，进程暂停
        else if (next->signals & SIGWAIT)
        {
            // 从调度表中移除当前进程
            task_list_delete(&(next->node_shedule));
            // 向等待进程表中加入当前进程
            task_list_add(&task_wait_list, &(next->node_shedule));
            // 修改进程状态
            next->status = INTERRUPTIBLE;
            next->signals ^= SIGWAIT;
            goto find;
        }
        // 收到SIGCHILD信号，若进程处于等待状态，则恢复
        // 正常情况下，处于等待状态的根本不会在这里被遍历到
        else if (next->signals & SIGCHILD)
        {
            if (next->status == INTERRUPTIBLE)
            {
                // 从等待链表中移除当前进程
                task_list_delete(&(next->node_shedule));
                // 向调度表中加入当前进程
                task_list_add(&task_schedule_list, &(next->node_shedule));
                // 修改进程状态
                next->status = RUNNING;
                next->signals ^= SIGCHILD;
            }
        }
        // 收到自定义的保留信号，调用自定义的信号处理函数
        else if (next->signals & SIGRESERVE)
        {
            sig_handler f = next->handler;
            f();
            next->signals ^= SIGRESERVE;
        }
    }
    finded:
    // if (next->pid == 1)
    // {
    //     kernel_printf("%x\n", next->context.epc);
    // }
    // kernel_printf("%s\n", next->name);
    
    if (next != current)
    {
        // kernel_printf("\nBefore switch_ex:\n");
        // kernel_printf("0x%x ", current->context.epc);
        // kernel_printf("%x ", *((uint*)(current->context.epc)));
        // kernel_printf("%x \n\n", *((uint*)(current->context.epc) + 1));
        // 上下文切换
        copy_context(pt_context, &(current->context));
        kernel_printf("\nBefore activateMemory:\n");
        kernel_printf("0x%x ", current->context.epc);
        kernel_printf("%x ", *((uint*)(current->context.epc)));
        kernel_printf("%x \n\n", *((uint*)(current->context.epc) + 1));
        // 激活地址空间
        if (next->mm)
        {
            activateMemory(next->mm);
        }
        if (current->pid == 0)
        {
            ((uint*)(current->context.epc))[0] = 0x3c08bfc0;
            ((uint*)(current->context.epc))[1] = 0x35089008;
            ((uint*)(current->context.epc))[2] = 0x3c09cafe;
            ((uint*)(current->context.epc))[3] = 0x3529babe;
            ((uint*)(current->context.epc))[4] = 0xad090000;
            ((uint*)(current->context.epc))[5] = 0x00000000;
            ((uint*)(current->context.epc))[6] = 0x00000000;
        }
        current = next;
        copy_context(&(current->context), pt_context);
        kernel_printf("After switch_ex:\n");
        kernel_printf("0x%x ", current->context.epc);
        kernel_printf("%x ", *((uint*)(current->context.epc)));
        kernel_printf("%x \n\n", *((uint*)(current->context.epc) + 1));
        if (current->pid == 0)
            disable_interrupts();
        // while (1);
    }
    }
    asm volatile("mtc0 $zero, $9\n\t");
}

// 遍历输出链表
void list_print(struct task_list_node* node)
{
    struct task_list_node* head = node;
    node = node->next;
    while (node != head)
    {
        kernel_printf("%x ", node);
        node = node->next;
    }
    kernel_printf("\n");
}

// 反向遍历输出链表
void list_print_reverse(struct task_list_node* node)
{
    struct task_list_node* head = node;
    node = node->last;
    while (node != head)
    {
        kernel_printf("%x ", node);
        node = node->last;
    }
    kernel_printf("\n");
}

// 寻找下一个运行的进程
struct task_struct* find_next_task()
{
    struct task_list_node* next = task_schedule_list.next;
    return find_task(next);
}

// 根据节点地址找到进程描述符的地址
struct task_struct* find_task(struct task_list_node* node)
{
    struct task_struct* x = container_of(node, struct task_struct, node);
    return x;
}

// 根据进程号找到进程描述符的地址
struct task_struct* find_task_pid(struct task_list_node* head, pid_t pid)
{
    struct task_list_node* node = head->next;
    while (node != head)
    {
        struct task_struct* task = container_of(node, struct task_struct, node);
        if (task->pid == pid)
        {
            return task;
        }
        node = node->next;
    }
    return (struct task_struct*)0;
}

// 0号进程
void idle()
{
    while (1);
}

// 为新进程分配一个尚未被分配的进程号
pid_t pid_alloc()
{
    // 遍历位图
    for (int i = 0; i < PID_STATUS_SIZE; ++i)
    {
        // 该行有空闲的pid号
        if (0xff != pid_status[i])
        {
            unsigned char tmp = pid_status[i];
            int j = 0;
            while (tmp % 2)
            {
                tmp /= 2;
                ++j;
            }
            // j为空闲pid所在的列
            pid_status[i] |= 1 << j;
            return (pid_t)(i * 8 + j);
        }
    }
    // 没有可用的pid
    return (pid_t)-1;
}

// 从pid位图中删除一个pid
void pid_delete(pid_t pid)
{
    int index = pid / 8;
    int offset = pid % 8;
    pid_status[index] = pid_status[index] | ~(1 << offset);
}

// 初始化任务链表，将头节点的首尾指向自身
void task_list_init(struct task_list_node* node)
{
    node->last = node;
    node->next = node;
}

// 向任务链表的尾部添加节点
void task_list_add(struct task_list_node* list, struct task_list_node* node)
{
    struct task_list_node* next = list, *prev = list->last;

    next->last = node;
	node->next = next;
	node->last = prev;
	prev->next = node;
}

// 从进程链表中删除节点
void task_list_delete(struct task_list_node* node)
{
    node->last->next = node->next;
    node->next->last = node->last;
}

// 寄存器上下文信息拷贝
void copy_context(context* src, context* dst)
{
    dst->epc = src->epc;
    dst->at = src->at;
    dst->v0 = src->v0;
    dst->v1 = src->v1;
    dst->a0 = src->a0;
    dst->a1 = src->a1;
    dst->a2 = src->a2;
    dst->a3 = src->a3;
    dst->t0 = src->t0;
    dst->t1 = src->t1;
    dst->t2 = src->t2;
    dst->t3 = src->t3;
    dst->t4 = src->t4;
    dst->t5 = src->t5;
    dst->t6 = src->t6;
    dst->t7 = src->t7;
    dst->s0 = src->s0;
    dst->s1 = src->s1;
    dst->s2 = src->s2;
    dst->s3 = src->s3;
    dst->s4 = src->s4;
    dst->s5 = src->s5;
    dst->s6 = src->s6;
    dst->s7 = src->s7;
    dst->t8 = src->t8;
    dst->t9 = src->t9;
    dst->hi = src->hi;
    dst->lo = src->lo;
    dst->gp = src->gp;
    dst->sp = src->sp;
    dst->fp = src->fp;
    dst->ra = src->ra;
}

// 向进程发送信号
void kill(pid_t pid, enum signal sig)
{
    // 从运行进程表中找进程
    struct task_struct* task = find_task_pid(&task_list, pid);
    if (task)
    {
        // 设置进程信号位
        task->signals |= sig;
    }
    // 从等待进程表中找进程
    else
    {
        task = find_task_pid(&task_wait_list, pid);
        task->signals |= sig;
        task_list_delete(&(task->node_shedule));
        task_list_add(&task_schedule_list, &(task->node_shedule));
    }
}

// 注册信号处理函数
void sigHandler(enum signal sig, sig_handler handler)
{
    pid_t pid = current->pid;
    struct task_struct* task = find_task_pid(&task_list, pid);
    if (sig == SIGRESERVE)
    {
        task->handler = handler;
    }
}

// 进程进入阻塞状态，等待子进程信号
void waitpid(pid_t pid)
{
    task_list_delete(&(current->node_shedule));
    task_list_add(&task_wait_list, &(current->node_shedule));
    current->status = INTERRUPTIBLE;
    
    // 以下注释掉的代码记录了一个极其诡异的debug过程
    // vm.c中有一个 volatile int i = 0
    // test_tlb_refill函数做完了，必须加这么一句代码
    // int a = enable_interrupts();
    // kernel_printf("old %d\n", a);
    // asm volatile("mtc0 $zero, $9\n\t");
    // asm volatile(
    //     "li $v0, 1000000\n\t"
    //     "mtc0 $v0, $11\n\t"
    //     "mtc0 $zero, $9");
    enable_interrupts();
    while (current->status == INTERRUPTIBLE)
    {
        // int x, y;
        // asm volatile(
        // "mfc0 %1, $11\n\t"
        // "mfc0 %0, $9\n\t"
        // :"=r"(x), "=r"(y));
        // kernel_printf("%d %d\n", x, y);
    }
}

// 获取9号寄存器和11号寄存器的内容
void getTimeCP0()
{
    int x, y;
    asm volatile(
        "mfc0 %1, $11\n\t"
        "mfc0 %0, $9\n\t"
        :"=r"(x), "=r"(y));
    kernel_printf("%d %d\n", x, y);
}

// 清除进程退出链表
void clearup()
{
    struct task_list_node* node = task_exit_list.next;
    while (node != &task_exit_list)
    {
        struct task_struct* task_exit = container_of(node, struct task_struct, node_shedule);
        task_list_delete(node);
        task_list_delete(&(task_exit->node));
        kfree(task_exit);
    }
}

// 加载用户程序使之成为进程
void loadUserProgram(char* fileName)
{
    int old = disable_interrupts();
    // 首先分配一个pid号
    pid_t pid = pid_alloc();
    // 创建内存描述符
    struct mm_struct* mm = create_mm_struct(pid);
    // 激活地址空间
    activateMemory(mm);
    // 将外存上的文件映射到内存，获取PC值
    void* epc = mmap(fileName);
    // kernel_printf("test5\n");
    activateMemory(current->mm);
    kernel_printf("load epc: %x\n", (unsigned int)epc);
    // 创建进程
    task_create_(pid, fileName, epc, USER, mm);
    // 交调度器定于一尊
    if (old)
        enable_interrupts();
    asm volatile("mtc0 $zero, $9\n\t");
}

#pragma GCC pop_options