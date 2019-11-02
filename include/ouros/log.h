#ifndef OUROS_LOG_H
#define OUROS_LOG_H

#include <driver/vga.h>

typedef enum {
    LOG_OK, LOG_FAIL, LOG_START, LOG_END, LOG_STEP
} LOG_STATE;

int log(int status, const char *format, ...);

#endif // OUROS_LOG_H