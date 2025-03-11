#include <lcom/lcf.h>
#include "keyboard.h"

#include "i8254.h"

int hook_id = 0;
uint8_t scancode;

int (kbc_subscribe_int)(uint8_t *bit_no) {
    hook_id = *bit_no;
    if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id) != OK) {
        printf("Error in sys_irqsetpolicy()\n");
        return 1;
    }
    return 0;
}

int (kbc_unsubscribe_int)() {
    if (sys_irqrmpolicy(&hook_id) != OK) {
        printf("Error in sys_irqrmpolicy()\n");
        return 1;
    }
    return 0;
}

int (kbc_send_cmd)(uint8_t cmd) {
    uint8_t stat;
    while (true) {
        util_sys_inb(KBC_ST_REG, &stat);
        if ((stat & KBC_ST_IBF) == 0) {
            sys_outb(KBC_CMD_REG, cmd);
            return 0;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }
}

int (kbc_data_handler)() {
    uint8_t data;
    uint8_t stat;
    while (true) {
        util_sys_inb(KBC_ST_REG, &stat);
        if(stat & KBC_ST_OBF) {
            util_sys_inb(KBC_OUT_BUF, &data);
            if ( (stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0 ) {
                return data;
            }
            else {
                return 1;
            }
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }
}

void (kbc_ih)() {
    scancode = kbc_data_handler();
}
