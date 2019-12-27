#include "vga.h"
#include <arch.h>

#include <ouros/time.h>

const uint BLANK = 0x000fff00;

CursorInfo cursor = {
    0, 0, 31
};

void init_vga() {
    cursor.row = cursor.col = 0;
    cursor.freq = 31;
    kernel_set_cursor();
}

// 光标信息：0x__AABBCC --> AA: 闪烁频率，BB：光标行数，CC：光标列数
void kernel_set_cursor() {
    *GPIO_CURSOR = ((cursor.freq & 0xff) << 16) 
                    + ((cursor.row & 0xff) << 8) 
                    + (cursor.col & 0xff);
}

void kernel_clear_screen() {
    cursor.col = 0;
    cursor.row = 0;
    kernel_set_cursor();
    // kernel_memset_uint(CHAR_VRAM, BLANK, 31 * VGA_SCREEN_MAX_COL);
    kernel_memset_uint(
        CHAR_VRAM, 
        BLANK, 
        VGA_SCREEN_MAX_ROW * VGA_SCREEN_MAX_COL
    );
}

void kernel_scroll_screen() {
    uint offset = VGA_COMMAND_MAX_ROW * VGA_SCREEN_MAX_COL;
    kernel_memcpy(
        CHAR_VRAM, 
        CHAR_VRAM + VGA_SCREEN_MAX_COL, 
        offset * sizeof(uint)
    );
    kernel_memset_uint(
        // CHAR_VRAM + (VGA_DISPLAY_MAX_ROW - 2) * VGA_SCREEN_MAX_COL, 
        CHAR_VRAM + offset, 
        BLANK, 
        VGA_SCREEN_MAX_COL
    );
}

int kernel_putchar(char ch)
{
    kernel_putch(ch, VGA_WHITE, VGA_BLACK);
}

int kernel_puts(const char* string)
{
    kernel_putstring(string, VGA_WHITE, VGA_BLACK);
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


int kernel_printf_color(int fgColor, int bgColor, const char *format, ...) {
    va_list argList;
    va_start(argList, format);

    int cnt = kernel_printf_argList(fgColor, bgColor, format, argList);

    va_end(argList);
    return cnt;
}

int kernel_printf_argList(int fgColor, int bgColor, const char* format, va_list argList)
{
    int cnt = 0;
    while (*format) {
        if (*format != '%') {
            kernel_putch(*format++, fgColor, bgColor);
        } else {
            char type = *++format;
            if(type == 'c') {
                char valch = va_arg(argList, int);
                kernel_putch(valch, fgColor, bgColor);
            } else if(type == 'd') {
                int valint = va_arg(argList, int);
                kernel_putint(valint, fgColor, bgColor);
            } else if(type == 'x' || type == 'X') {
                uint valint = va_arg(argList, uint);
                kernel_puthex(valint, type == 'X', fgColor, bgColor);
            } else if(type == 's') {
                char *valstr = va_arg(argList, char*);
                kernel_putstring(valstr, fgColor, bgColor);
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

int kernel_putch(int ch, int fgColor, int bgColor) {
    uint* cursor_addr = (uint*)(CHAR_VRAM + cursor.row * VGA_SCREEN_MAX_COL + cursor.col);
    
    if (ch == '\r')
        ;
    else if (ch == '\n') {
        // kernel_memset_uint(cursor_addr, BLANK, VGA_DISPLAY_MAX_COL - cursor.col);
        cursor.col = 0;
        if (cursor.row == VGA_COMMAND_MAX_ROW) {
            kernel_scroll_screen();
        } else {
            cursor.row ++;
#ifdef VGA_CALIBRATE
            kernel_putch(' ', fgColor, bgColor);
#endif  // VGA_CALIBRATE
        }
    } else if (ch == '\t') {
        if (cursor.col >= VGA_DISPLAY_MAX_COL - 4) {
            kernel_putch('\n', VGA_BLACK, VGA_BLACK);
        } else {
            kernel_memset_uint(cursor_addr, BLANK, 4 - cursor.col & 0x3);
            // tab 4 制表位对齐
            cursor.col = (cursor.col + 4) & (-0x4);
        }
    } else {
        if (cursor.col == VGA_DISPLAY_MAX_COL) {
            kernel_putch('\n', 0, 0);
        }
        kernel_putchar_at_color(ch, fgColor, bgColor, cursor.row, cursor.col);
        cursor.col++;
    }

    kernel_set_cursor();
    return ch;
}

int kernel_putstring(const char *string, int fgColor, int bgColor) {
    int ret = 0;
    while (*string) {
        ret ++;
        kernel_putch(*string++, fgColor, bgColor);
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
        kernel_putchar('0');
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
        kernel_putch('0', fgColor, bgColor);
    } else {
        while (hex) {
            char value = hex & 0xF;
            ptr--;
            *ptr = HEX_MAP[value] + ((isUpper && value > 9)? ('A' - 'a'): 0);
            hex >>= 4;
        }
        kernel_putstring(ptr, fgColor, bgColor);
    }
    return hex;
}

void kernel_putchar_at(int ch, int row, int col) {
    kernel_putchar_at_color(ch, VGA_WHITE, VGA_BLACK, row, col);
}

// 字符数据信息：0xBBBFFFCC --> BBB：背景颜色，FFF: 前景颜色，CC：字符值
void kernel_putchar_at_color(int ch, int fgColor, int bgColor, int row, int col) {
    row = row & 0x1F;
    col = col & 0x7F;

    // max col = 128
    uint *p = CHAR_VRAM + (row << 7) + col;
    *p = ((bgColor & 0xFFF) << 20) + ((fgColor & 0xFFF) << 8) + (ch & 0xFF);
}



