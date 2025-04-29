#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <minix/sysutil.h>
#include <lcom/lcf.h>
#include "i8042.h"
#include "kbc.h"

int (keyboard_subscribe_int)(uint8_t *bit_no);
int (keyboard_unsubscribe_int)();
void (keyboard_process_scancode)();
void (kbc_ih)();
int (keyboard_restore)();

#endif //KEYBOARD_H
