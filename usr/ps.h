#ifndef PS_H
#define PS_H

#include <os/type.h>

void ps();
void ps_parse_cmd();

void callback(int keyCode, bool pressDown);

#endif // PS_H
