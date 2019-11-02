#ifndef DRIVER_PS2
#define DRIVER_PS2

#include <intr.h>
#include <ouros/pc.h>

#define KEY_CAPS_LOCK       0x58
#define KEY_LEFT_SHIFT      0x12
#define KEY_RIGHT_SHIFT     0x59
#define KEY_LEFT_CTRL       0x14
#define KEY_RIGHT_CTRL      0xE014
#define KEY_LEFT_ALT        0x11
#define KEY_RIGHT_ALT       0xE011

#define KEY_ENTER 			0x5A
#define KEY_TAB				0x0D
#define KEY_SPACE 			0x29
#define KEY_ESC 			0x76
#define KEY_BACKSPACE 		0x66

#define KEY_ARROW_UP        0xE075
#define KEY_ARROW_DOWN     	0xE072
#define KEY_ARROW_LEFT     	0xE06B
#define KEY_ARROW_RIGHT     0xE074

#define KEY_F1				0x05
#define KEY_F2				0x06
#define KEY_F3				0x04
#define KEY_F4				0x0C
#define KEY_F5				0x03
#define KEY_F6				0x0B
#define KEY_F7				0x83
#define KEY_F8				0x0A
#define KEY_F9				0x01
#define KEY_F10				0x09
#define KEY_F11				0x78
#define KEY_F12				0x07

void init_ps2();
void ps2_handler(unsigned int status, unsigned int cause, context* pt_context);
int kernel_getkey();
int kernel_getchar();

typedef void (*KeyBoard_Callback)(int keyCode, bool pressDown);
void register_keyboard_callback(KeyBoard_Callback callback);

#endif // DRIVER_PS2