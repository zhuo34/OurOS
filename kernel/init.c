#include <arch.h>
#include <exc.h>
#include <intr.h>
#include <tlb.h>

#include <driver/ps2.h>
#include <driver/vga.h>
#include <ouros/time.h>

#include <ouros/log.h>

#include <ouros/pc.h>
#include <ouros/mm.h>
// #include <ouros/slab.h>
// #include <ouros/bootmm.h>
// #include <ouros/buddy.h>
#include <ouros/vm.h>

#include <ouros/fs/fs.h>

#include <ouros/syscall.h>

#include "../usr/ps.h"
#include "../usr/shell.h"
#pragma GCC push_options
#pragma GCC optimize("O0")
void machine_info() {
    int row;
    int col;
    kernel_printf("\n%s\n", "412-UNIX V1.0");
    row = cursor.row;
    col = cursor.col;
    cursor.row = VGA_DISPLAY_MAX_ROW - 1;
    kernel_printf("%s", "Created by Dorm 412 Block 32, Yuquan Campus, Zhejiang University.");
    cursor.row = row;
    cursor.col = col;
    kernel_set_cursor();
}

void test3()
{
    // kernel_printf("testSIG\n");
    // while (1)
}

void test2()
{
    kernel_printf("test6\n");
    
    test_tlb_refill(1926);
    // int x, y;
    // asm volatile(
    //     "mfc0 %1, $11\n\t"
    //     "mfc0 %0, $9\n\t"
    //     :"=r"(x), "=r"(y));
    // kernel_printf("%d %d\n", x, y);
    kill(current->parent, SIGCHILD);
    kernel_printf("test7\n");
    // pid_t pid2 = task_create("test", test1926, 0, (void*)0, USER);
    // kill(current->parent, SIGWAIT);
    // for (int i = 0 ; i < 1000; ++i)
    // {
    //     if (!(i % 100))
    //         kernel_printf("%d\n", i);
    // }
    // kill(current->parent, SIGCHILD);
    // test_tlb_refill(1999);
    while (1)
    {
        // kernel_printf("test6\n");
    }
    
    // while (1);

    // test_tlb_refill();
}
void test5()
{
    kernel_printf("test begin\n");
    // void* epc = mmap("b.bin");
    kernel_printf("laod file SUCCEDED\n");
    *GPIO_SEG = 0x87654321;
    // asm volatile(
    //     "lui $t0, 0xbfc0\n\t"
    //     "ori $t0, $t0, 0x9008\n\t"
    //     "lui $t1, 0x3344\n\t"
    //     "ori $t1, $t1, 0x5566\n\t"
    //     "sw $t1, 0($t0)"
    // );
    while (1);
}
void test1()
{
    kernel_printf("test\n");
    // sigHandler(SIGRESERVE, test3);
    // test_tlb_refill(5575);
    
    // for (int i = 0 ; i < 1000; ++i)
    // {
    //     if (!(i % 100))
    //         kernel_printf("%d\n", i);
    // }
    test_tlb_refill(5555);
    // test_tlb_refill(1000);
    
    // kernel_printf("qiguai\n");
    // 什么鬼？
    // int x, y;
    // asm volatile(
    //     "mfc0 %1, $11\n\t"
    //     "mfc0 %0, $9\n\t"
    //     :"=r"(x), "=r"(y));
    // kernel_printf("%d %d\n", x, y);
    
    
    // for (int i = 0; i < 1000; ++i)
    // {
    //     // if (!(i % 200))
    //     //     kernel_printf("%d\n", i);
    // }
    // asm volatile(
    //     "mfc0 %1, $11\n\t"
    //     "mfc0 %0, $9\n\t"
    //     :"=r"(x), "=r"(y));
    // kernel_printf("%d %d\n", x, y);
    pid_t pid2 = task_create("test", test2, 0, (void*)0, USER);
    waitpid(pid2);
    int *test_vaddrs[4];

	for (int i = 0; i < 4; i++) {
		test_vaddrs[i] = (int *)(0x08000000 + 0x00004000 * i);
		kernel_printf("access %x\n", test_vaddrs[i]);
		// int a = val + i;
		// *(test_vaddrs[i]) = a;
		kernel_printf("test %d\n", *(test_vaddrs[i]));
	}
    // waitpid();
    pid_t pid3 = task_create("test", test5, 0, (void*)0, USER);
    while(1)
    {
        // kernel_printf("test\n");
    }
    // while (1);

    
    // while (1)
}

void test()
{
    pid_t pid = task_create("test", test1, 0, (void*)0, USER);
    
    
    // kernel_printf("%d\n", pid);
    // while (1);
}

void init_kernel() {
    // init_done = 0;
    kernel_clear_screen();
    // Exception
    init_exception();
    // Drivers
    init_vga();
    init_ps2();
    init_time();

    // tlb
    init_tlb();
    // Memory management
    log(LOG_START, "Memory Modules.");
    init_bootmm();
    log(LOG_OK, "Bootmem.");
    init_buddy();
    log(LOG_OK, "Buddy.");
    init_slab();
    log(LOG_OK, "Slab.");
    log(LOG_END, "Memory Modules.");
    // test_tlb_refill();
    // while (1);

    // File system
    log(LOG_START, "File System.");
    init_fs();
    log(LOG_END, "File System.");

    // System call
    // log(LOG_START, "System Calls.");
    // init_syscall();
    // log(LOG_END, "System Calls.");

    // Process control
    log(LOG_START, "Process Control Module.");
    init_pc();
    log(LOG_END, "Process Control Module.");
    // Interrupts
    log(LOG_START, "Enable Interrupts.");
    init_interrupts();
    log(LOG_END, "Enable Interrupts.");
    *GPIO_SEG = 0x12345678;
    // loadUserProgram("bb.bin");
    task_create("test_GPIO", test5, 0, 0, USER);
    // // Init finished
    // machine_info();
    // *GPIO_SEG = 0x11223344;
    // asm volatile(
    //     "lui $t0,0x8001\n\t"
    //     "lw	$t0,12016($t0)\n\t"
    //     "lui $t1,0x1122\n\t"
    //     "ori $t1,$t1,0x3344\n\t"
    //     "sw	$t1,0($t0)\n\t"
    // );
    // uint a = *GPIO_SWITCH;
    // test();
    // ps();
    // osh();
    // while (1);
    osh();
}
#pragma GCC pop_options