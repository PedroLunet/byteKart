#include "keyboard.h"

int hook_id_keyboard = 1;
uint8_t scancode = 0;

uint8_t bytes[2];
uint8_t size = 0;
bool make = true;

int (keyboard_subscribe_int)(uint8_t *bit_no) {
    if (bit_no == NULL) return 1;
    *bit_no = BIT(hook_id_keyboard);
    if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_keyboard) != 0) {
        return 1;
    }
    return 0;
}

int (keyboard_unsubscribe_int)() {
    if (sys_irqrmpolicy(&hook_id_keyboard) != 0) {
        return 1;
    }
    return 0;
}

void (keyboard_process_scancode)() {
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
    if (kbc_read_output(KBC_OUT_CMD, &scancode, 0) != 0) {
        printf("Error: Could not read scancode!\n");
    }
}

int (keyboard_restore)() {

    uint8_t command_byte;

    if (kbc_write_command(KBC_IN_CMD, KBC_READ_CMD) != 0) {
        return 1;
    }

    if (kbc_read_output(KBC_OUT_CMD, &command_byte, 0) != 0) {
        return 1;
    }

    command_byte |= ENABLE_INT;

    if (kbc_write_command(KBC_IN_CMD, KBC_WRITE_CMD) != 0) {
        return 1;
    }

    if (kbc_write_command(KBC_WRITE_CMD, command_byte) != 0) {
        return 1;
    }

    return 0;
}

