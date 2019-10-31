#ifndef STRING_H
#define STRING_H
#include <driver/ps2.h>
#include <driver/sd.h>
#include <driver/vga.h>
#include <os/bootmm.h>
#include <os/buddy.h>
#include <os/slab.h>
#include <os/time.h>
#include <os/utils.h>

// 从标准输入获取一行
bool read_line(char* str, int length);

#endif
