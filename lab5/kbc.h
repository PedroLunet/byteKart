#ifndef _KBC_LAB3_H_
#define _KBC_LAB3_H_

#include "i8042.h"
#include "i8254.h"

int (write_command_KBC)(uint8_t port, uint8_t command);
int (read_output_KBC)(uint8_t port, uint8_t *output, uint8_t mouse);
int (kbc_subscribe_int)(uint8_t *bit_no);
int (kbc_unsubscribe_int)();
void (kbc_ih_handler)();


#endif
