#include <assert.h>
#include <os/log.h>
#include <os/time.h>
#include <driver/vga.h>

#define MAX_LEVEL 4
#define MIN_LEVEL 0
static int log_level = 0;

int gstep = 0;

void log_level_adv() {
    if (log_level < MAX_LEVEL) {
        log_level++;
    }
}

void log_level_rec() {
    if (log_level > MIN_LEVEL) {
        log_level--;
    }
}

void log(int status, const char *format, ...) {
    char time_buf[9];
    // print status
    switch (status) {
        case LOG_OK:
            kernel_putstring("[ O K ] ", VGA_GREEN, VGA_BLACK);
            break;
        case LOG_FAIL:
            kernel_putstring("[FAIL] ", VGA_RED, VGA_BLACK);
            break;
        case LOG_START:
            kernel_putstring("[START] ", VGA_BLUE, VGA_BLACK);
            break;
        case LOG_END:
            kernel_putstring("[ END ] ", VGA_BLUE, VGA_BLACK);
            break;
        case LOG_STEP:
            kernel_putstring("[STEP] ", VGA_WHITE, VGA_BLACK);
            break;
        default:
            kernel_assert(0, "[LOG]: Undefined log status.");
            break;
    }
    // print time
    get_time(time_buf, sizeof(time_buf));
    kernel_printf(" %s ", time_buf);
    // print log message
    va_list ap;
    va_start(ap, format);
    kernel_printf_argList(VGA_WHITE, VGA_BLACK, format, ap);
    va_end(ap);
    kernel_printf("\n");
}

void step() {
    gstep++;
    log(LOG_STEP, "%x", gstep);
}

void step_reset() {
    gstep = 0;
}