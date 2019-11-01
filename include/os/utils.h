#ifndef OS_UITILS_H
#define OS_UITILS_H

#include <os/type.h>

#define upper_align(x, y) (((x)+((y)-1)) & ~((y)-1))
#define max(x, y) (((x)>(y))?(x):(y))
#define min(x, y) (((x)<(y))?(x):(y))

#define container_of(ptr, type, member) ((type*)((char*)ptr - (char*)&(((type*)0)->member)))

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

void* kernel_memcpy(void* dst, void* src, int len);
void* kernel_memset(void* dst, uchar data, uint len);
uint* kernel_memset_uint(uint* dst, uint value, int len);
int kernel_strcmp(const char* dst, const char* src);
int pow(int x, int z);
void kernel_cache(unsigned int block_index);
char* kernel_strcpy(char* dst, const char* src);
void kernel_serial_puts(char* str);
void kernel_serial_putc(char c);
unsigned int is_bound(unsigned int val, unsigned int bound);
uint get_low_bits(uint src, uint n_bit);

typedef unsigned char* va_list;
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(unsigned int) - 1) & ~(sizeof(unsigned int) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) (ap = (va_list)0)

#endif // OS_UITILS_H
