#ifndef _MOUSE_LAB4_H_
#define _MOUSE_LAB4_H_

#include "i8042.h"
#include "i8254.h"
#include "kbc.h"

int (mouse_subscribe_int)(uint8_t *bit_no);
int (mouse_unsubscribe_int)();
void (mouse_ih)();

#endif
