#ifndef STRING_H
#define STRING_H
#include <driver/ps2.h>
#include <driver/sd.h>
#include <driver/vga.h>
#include <ouros/bootmm.h>
#include <ouros/buddy.h>
#include <ouros/slab.h>
#include <ouros/time.h>
#include <ouros/utils.h>

// 从标准输入获取一行
bool read_line(char* str, int length);

#endif
