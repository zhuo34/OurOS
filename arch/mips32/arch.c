#include "arch.h"

uint* const CHAR_VRAM = (uint*)0xbfc04000;
uint* const GRAPHIC_VRAM = (uint*)0xbfe0000;
uint* const GPIO_SWITCH = (uint*)0xbfc09000;     // switch read-only
uint* const GPIO_BUTTON = (uint*)0xbfc09004;     // button read-only
uint* const GPIO_SEG = (uint*)0xbfc09008;        // Seg R/W
uint* const GPIO_LED = (uint*)0xbfc0900c;        // LED R/W
uint* const GPIO_PS2_DATA = (uint*)0xbfc09010;   // PS/2 data register, R/W
uint* const GPIO_PS2_CTRL = (uint*)0xbfc09014;   // PS/2 control register, R/W
uint* const GPIO_UART_DATA = (uint*)0xbfc09018;  // UART data register, R/W
uint* const GPIO_UART_CTRL = (uint*)0xbfc0901c;  // UART control register, R/W
uint* const GPIO_CURSOR = (uint*)0xbfc09020;     // Cursor 8-bit frequency 8-bit row 8-bit col
uint* const VGA_MODE = (uint*)0xbfc09024;        // enable graphic mode

volatile uint kernel_sp = 0x81000000;

uint get_phymm_size() {
    return MACHINE_MMSIZE;
}