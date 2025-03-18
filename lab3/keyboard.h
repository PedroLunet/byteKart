#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <lcom/lcf.h>

int (kbc_subscribe_int)(uint8_t *bit_no);
int (kbc_unsubscribe_int)();
void (kbc_ih)();
int (kbc_write)(uint8_t status, uint8_t st_ibf, uint8_t port);
int (kbc_read_output_buffer)();
void (kbd_process_scancode)();

#endif //KEYBOARD_H
