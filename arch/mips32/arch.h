#ifndef ARCH_H
#define ARCH_H

#include <ouros/type.h>

// machine parameters
#define MACHINE_MMSIZE (128 * 1024 * 1024)		// 128MB
#define MACHINE_SDSIZE (32 * 1024 * 1024)		// 32M Sectors
// #define PAGE_TABLE_SIZE 256 * 1024           // 4MB
#define CHAR_VRAM_SIZE (128 * 32 *4)			// 128 * 32 * 4
#define GRAPHIC_VRAM_SIZE (1024 * 512 * 4)		// 1024 * 512 * 4 b-g-r

//	Virtual Memory
#define BIOS_ENTRY 0xbfc00000
#define KERNEL_STACK_BOTTOM 0x81000000
#define KERNEL_CODE_ENTRY 0x80001000
#define KERNEL_ENTRY 0x80000000
#define USER_ENTRY 0x00000000

extern uint* const CHAR_VRAM;
extern uint* const GRAPHIC_VRAM;
extern uint* const GPIO_SWITCH;     // switch read-only
extern uint* const GPIO_BUTTON;     // button read-only
extern uint* const GPIO_SEG;        // Seg R/W
extern uint* const GPIO_LED;        // LED R/W
extern uint* const GPIO_PS2_DATA;   // PS/2 data register, R/W
extern uint* const GPIO_PS2_CTRL;   // PS/2 control register, R/W
extern uint* const GPIO_UART_DATA;  // UART data register, R/W
extern uint* const GPIO_UART_CTRL;  // UART control register, R/W
extern uint* const GPIO_CURSOR;     // Cursor 8-bit frequency 8-bit row 8-bit col
extern uint* const VGA_MODE;        // enable graphic mode

// kernel sp
extern volatile uint kernel_sp;

uint get_phymm_size();

#endif // ARCH_H
