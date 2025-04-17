#ifndef KBC_H
#define KBC_H

#include <minix/sysutil.h>
#include "i8042.h"

int (kbc_read_status)(uint8_t* status);
int (kbc_write_command)(uint8_t port, uint8_t command);
int (kbc_read_output)(uint8_t port, uint8_t *data, uint8_t mouse);

#endif //KBC_H
