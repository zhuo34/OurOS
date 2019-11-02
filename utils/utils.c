#include <driver/vga.h>
#include <ouros/utils.h>

void* kernel_memcpy(void* dst, void* src, int len) {
    char* dststr = dst;
    char* srcstr = src;
    while (len--) {
        *dststr = *srcstr;
        dststr++;
        srcstr++;
    }
    return dst;
}

#pragma GCC push_options
#pragma GCC optimize("O2")
void* kernel_memset(void* dst, uchar data, uint len) {
    uchar *p_dst = dst;
    while (len--) {
        *p_dst++ = data;
    }
    return dst;
}
#pragma GCC pop_options

uint* kernel_memset_uint(uint* dst, uint value, int len) {
    while (len--)
        *dst++ = value;

    return dst;
}

int kernel_strcmp(const char* dst, const char* src) {
    while ((*dst == *src) && (*dst != 0)) {
        dst++;
        src++;
    }
    return *dst - *src;
}

char* kernel_strcpy(char* dst, const char* src) {
    while ((*dst++ = *src++))
        ;
    return dst;
}

int pow(int x, int z) {
    int ret = 1;
    if (z < 0)
        return -1;
    while (z--) {
        ret *= x;
    }
    return ret;
}

#pragma GCC push_options
#pragma GCC optimize("O0")

void kernel_cache(unsigned int block_index) {
    // block_index = block_index | 0x80000000;
    asm volatile(
        "li $t0, 233\n\t"
        "mtc0 $t0, $8\n\t"
        "move $t0, %0\n\t"
        "cache 0, 0($t0)\n\t"
        "nop\n\t"
        "cache 1, 0($t0)\n\t"
        "nop\n\t"
        :
        : "r"(block_index)
    );
}

#pragma GCC pop_options

void kernel_serial_puts(char* str) {
    while (*str)
        *((unsigned int*)0xbfc09018) = *str++;
}

void kernel_serial_putc(char c) {
    *((unsigned int*)0xbfc09018) = c;
}

unsigned int is_bound(unsigned int val, unsigned int bound) {
    return !(val & (bound - 1));
}

uint get_low_bits(uint src, uint n_bit)
{
    if (n_bit > 32) {
        return src;
    }
    uint mask = 0;
    for (uint i = 0; i < n_bit; i++) {
        mask = (mask << 1) + 1;
    }
    return src & mask;
}