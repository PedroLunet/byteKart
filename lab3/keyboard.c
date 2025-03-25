#include <lcom/lcf.h>
#include "keyboard.h"

#include "i8254.h"

int hook_id_keyboard = 0;
uint8_t scancode;

uint8_t bytes[2];
uint8_t size = 0;
bool make = true;

int count = 0;

int (kbc_subscribe_int)(uint8_t *bit_no) {
    hook_id_keyboard = *bit_no;
    if (sys_irqsetpolicy(KBC_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_keyboard) != OK) {
        printf("Error in sys_irqsetpolicy()\n");
        return 1;
    }
    return 0;
}

int (kbc_unsubscribe_int)() {
    if (sys_irqrmpolicy(&hook_id_keyboard) != OK) {
        printf("Error in sys_irqrmpolicy()\n");
        return 1;
    }
    return 0;
}

// status: status byte to be written
// free_buffer_mask: mask to check if the buffer is free
// port: port to write the status byte
int (kbc_write)(uint8_t input, uint8_t free_buffer_mask, uint8_t port) {
    uint8_t stat;
    while (true) {
        util_sys_inb(KBC_ST_REG, &stat);
        #ifdef LAB3
            count++;
        #endif
        if ((stat & free_buffer_mask) == 0) {
            sys_outb(port, input);
            return 0;
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }
}

int (kbc_read_output_buffer)() {
    uint8_t data;
    uint8_t stat;

    while (true) {
        util_sys_inb(KBC_ST_REG, &stat);
        #ifdef LAB3
            count++;
        #endif
        if(stat & KBC_ST_OBF) {
            util_sys_inb(KBC_OUT_BUF, &data);
            #ifdef LAB3
                count++;
            #endif
            if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0) {
                if (stat & KBC_ST_AUX) {
                    return (data | 0x100);
                } else {
                    return data;
                }
            }
            else {
                return 1;
            }
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }
}

void (kbd_process_scancode)() {
  if (bytes[0] != 0xE0) {
    bytes[0] = scancode;
    size = 1;
    if (scancode != 0xE0) {
      if (scancode & BREAK_BIT) {
        make = false;
      }
      kbd_print_scancode(make, size, bytes);
      make = true;
    }
  } else {
    bytes[1] = scancode;
    size = 2;
    if (scancode & BREAK_BIT) {
      make = false;
    }
  }
}

void (kbc_ih)() {

    uint8_t data = kbc_read_output_buffer();

    if ((data & 0x100) == 0) {
        // Data is from the keyboard
        scancode = data;
        kbd_process_scancode();
    }
}

