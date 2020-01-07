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
#include <ouros/shm.h>

#include <ouros/fs/fs.h>

#include <ouros/syscall.h>

#include "../usr/ps.h"
#include "../usr/shell.h"
#pragma GCC push_options
#pragma GCC optimize("O0")


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
	init_page_pool();

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

    osh();
}
#pragma GCC pop_options