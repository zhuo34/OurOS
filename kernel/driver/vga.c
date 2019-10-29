#include "vga.h"
#include <arch.h>
#include <os/utils.h>

const int VGA_SCREEN_MAX_ROW = 32;
const int VGA_SCREEN_MAX_COL = 128;
const int VGA_DISPLAY_MAX_ROW = 28;
const int VGA_DISPLAY_MAX_COL = 80;

const uint BLANK = 0x000fff00;

int cursor_row;
int cursor_col;
int cursor_freq = 31;

void init_vga() {
    cursor_row = cursor_col = 0;
    cursor_freq = 31;
    kernel_set_cursor();
}

// 光标信息：0x__AABBCC --> AA: 闪烁频率，BB：光标行数，CC：光标列数
void kernel_set_cursor() {
    *GPIO_CURSOR = ((cursor_freq & 0xff) << 16) 
                    + ((cursor_row & 0xff) << 8) 
                    + (cursor_col & 0xff);
}

void kernel_clear_screen() {
    cursor_col = 0;
    cursor_row = 0;
    kernel_set_cursor();
    // kernel_memset_uint(CHAR_VRAM, BLANK, 31 * VGA_SCREEN_MAX_COL);
    kernel_memset_uint(
        CHAR_VRAM, 
        BLANK, 
        VGA_SCREEN_MAX_ROW * VGA_SCREEN_MAX_COL
    );
}

void kernel_scroll_screen() {
    kernel_memcpy(
        CHAR_VRAM, 
        CHAR_VRAM + VGA_SCREEN_MAX_COL, 
        VGA_DISPLAY_MAX_ROW * VGA_SCREEN_MAX_COL * sizeof(int)
    );
    // kernel_memset_uint(
    //     // CHAR_VRAM + (VGA_DISPLAY_MAX_ROW - 2) * VGA_SCREEN_MAX_COL, 
    //     CHAR_VRAM + offset, 
    //     BLANK, 
    //     VGA_SCREEN_MAX_COL
    // );
}


int kernel_printf(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);

    int cnt = kernel_printf_argList(VGA_WHITE, VGA_BLACK, format, argList);

    va_end(argList);
    return cnt;
}

int kernel_printf_error(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);

    int cnt = kernel_printf_argList(VGA_RED, VGA_BLACK, format, argList);

    va_end(argList);
    return cnt;
}


int kernel_printf_color(int fgColor, int bgColorColor, const char *format, ...) {
    va_list argList;
    va_start(argList, format);

    int cnt = kernel_printf_argList(fgColor, bgColorColor, format, argList);

    va_end(argList);
    return cnt;
}

int kernel_printf_argList(int fgColor, int bgColorColor, const char* format, va_list argList)
{
    int cnt = 0;
    while (*format) {
        if (*format != '%') {
            kernel_putchar(*format++, VGA_WHITE, VGA_BLACK);
        } else {
            char type = *++format;
            if(type == 'c') {
                char valch = va_arg(argList, int);
                kernel_putchar(valch, VGA_WHITE, VGA_BLACK);
            } else if(type == 'd') {
                int valint = va_arg(argList, int);
                kernel_putint(valint, VGA_WHITE, VGA_BLACK);
            } else if(type == 'x' || type == 'X') {
                int valint = va_arg(argList, int);
                kernel_puthex(valint, type == 'X', VGA_WHITE, VGA_BLACK);
            } else if(type == 's') {
                char *valstr = va_arg(argList, char*);
                kernel_putstring(valstr, VGA_WHITE, VGA_BLACK);
            } else {
                cnt = -1;
                break;
            }
            format ++;
            cnt ++;
        }
    }

    return cnt;
}

int kernel_putchar(int ch, int fgColor, int bgColor) {
    uint* cursor_addr = (uint*)(CHAR_VRAM + cursor_row * VGA_SCREEN_MAX_COL + cursor_col);
    
    if (ch == '\r')
        ;
    else if (ch == '\n') {
        kernel_memset_uint(cursor_addr, BLANK, VGA_DISPLAY_MAX_COL - cursor_col);
        cursor_col = 0;
        if (cursor_row == VGA_DISPLAY_MAX_ROW) {
            kernel_scroll_screen();
        } else {
            cursor_row ++;
#ifdef VGA_CALIBRATE
            kernel_putchar(' ', fgColor, bgColor);
#endif  // VGA_CALIBRATE
        }
    } else if (ch == '\t') {
        if (cursor_col >= VGA_DISPLAY_MAX_COL - 4) {
            kernel_putchar('\n', VGA_BLACK, VGA_BLACK);
        } else {
            kernel_memset_uint(cursor_addr, BLANK, 4 - cursor_col & 0x3);
            // tab 4 制表位对齐
            cursor_col = (cursor_col + 4) & (-0x4);
        }
    } else {
        if (cursor_col == VGA_DISPLAY_MAX_COL) {
            kernel_putchar('\n', 0, 0);
        }
        kernel_putchar_at_color(ch, fgColor, bgColor, cursor_row, cursor_col);
        cursor_col++;
    }

    kernel_set_cursor();
    return ch;
}

int kernel_putstring(const char *string, int fgColor, int bgColor) {
    int ret = 0;
    while (*string) {
        ret ++;
        kernel_putchar(*string++, fgColor, bgColor);
    }
    return ret;
}

int kernel_putint(int num, int fgColor, int bgColor) {
    // int 最大值为10位数，加上符号最多11位，再加上结尾'\0' 1位
    // 故字符串buffer取12位
    char buffer[12];
    buffer[11] = '\0';
    char *ptr = buffer + 11;

    bool isNegative = false;
    if (num < 0) {
        isNegative = true;
        num = -num;
    }
    
    if (num == 0) {
        kernel_putchar('0', fgColor, bgColor);
    } else {
        while (num) {
            ptr--;
            *ptr = (num % 10) + '0';
            num /= 10;
        }
        if (isNegative) {
            ptr--;
            *ptr = '-';
        }

        kernel_putstring(ptr, fgColor, bgColor);
    }
    
    return num;
}

static const char *HEX_MAP = "0123456789abcdef";
int kernel_puthex(uint hex, bool isUpper, int fgColor, int bgColor) {
    char buffer[12];
    char *ptr = buffer + 11;
    buffer[11] = '\0';

    if (hex == 0) {
        kernel_putchar('0', fgColor, bgColor);
    } else {
        while (hex) {
            char value = hex & 0xF;
            ptr--;
            *ptr = HEX_MAP[value] + (isUpper && value > 9)? ('A' - 'a'): 0;
            hex >>= 4;
        }
        kernel_putstring(ptr, fgColor, bgColor);
    }
    return hex;
}

void kernel_putchar_at(int ch, int row, int col) {
    kernel_putchar_at_color(ch, VGA_BLACK, VGA_WHITE, row, col);
}

// 字符数据信息：0xBBBFFFCC --> BBB：背景颜色，FFF: 前景颜色，CC：字符值
void kernel_putchar_at_color(int ch, int fgColor, int bgColor, int row, int col) {
    row = row & 0x1F;
    col = col & 0x7F;

    uint *p = CHAR_VRAM + row * VGA_SCREEN_MAX_COL + col;
    *p = ((bgColor & 0xFFF) << 20) + ((fgColor & 0xFFF) << 8) + (ch & 0xFF);
}



