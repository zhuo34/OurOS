#ifndef DRIVER_VGA_H
#define DRIVER_VGA_H

#include <os/utils.h>

extern int cursor_row;
extern int cursor_col;
extern int cursor_freq;

#define VGA_RED     0x00f
#define VGA_GREEN   0x0f0
#define VGA_BLUE    0xf00
#define VGA_BLACK   0x000
#define VGA_WHITE   0xfff
#define VGA_YELLOW  0x0ff

// 提供给外部的接口
void init_vga();

void kernel_set_cursor();
void kernel_clear_screen();
void kernel_scroll_screen();

int kernel_printf(const char* format, ...);
int kernel_printf_error(const char* format, ...);
int kernel_printf_color(int fgColor, int bgColor, const char* format, ...);

void kernel_putchar_at(int ch, int row, int col);
void kernel_putchar_at_color(int ch, int fgColor, int bgColor, int row, int col);

// 内部使用
int kernel_printf_argList(int fgColor, int bgColor, const char* format, va_list argList);
int kernel_putchar(int ch, int fgColor, int bgColor);
int kernel_putstring(const char* string, int fgColor, int bgColor);
int kernel_putint(int num, int fgColor, int bgColor);
int kernel_puthex(uint hex, bool isUpper, int fgColor, int bgColor);


#endif // DRIVER_VGA_H