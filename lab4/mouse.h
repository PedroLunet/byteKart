#ifndef MOUSE_H
#define MOUSE_H

#include <minix/sysutil.h>
#include <lcom/lcf.h>
#include "i8042.h"
#include "../lab3/kbc.h"

int (mouse_subscribe_int)(uint8_t *bit_no);
int (mouse_unsubscribe_int)();
void (mouse_ih)();
void (mouse_process_scanbyte)();
int (mouse_write_command)(uint8_t command);
void (lcf_print_packet)(uint8_t *packet);

#endif //MOUSE_H
