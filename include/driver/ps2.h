#ifndef DRIVER_PS2
#define DRIVER_PS2

#include <intr.h>
#include <os/pc.h>

void init_ps2();
void ps2_handler(unsigned int status, unsigned int cause, context* pt_context);
int kernel_getkey();
int kernel_getchar();

typedef void (*KeyBoard_Callback)(int keyCode, bool isUp);
void register_keyboard_callback(KeyBoard_Callback callback);

#endif // DRIVER_PS2