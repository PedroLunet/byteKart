#ifndef _MOUSE_LAB4_H_
#define _MOUSE_LAB4_H_

#include "i8042.h"
#include "i8254.h"
#include "kbc.h"

int (mouse_subscribe_int)(uint8_t *bit_no);
int (mouse_unsubscribe_int)();
void (mouse_ih)();
int (mouse_data_reporting)(uint8_t command);
void (mouse_bytes)();
int (mouse_write_command)(uint8_t mouse_command);
void (mouse_struct_packet)(struct packet* pp);
void (state_machine)(uint8_t x_len, uint8_t tolerance);

#endif
