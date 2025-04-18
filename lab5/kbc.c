#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "kbc.h"

int hook_id_kbc = 1;
uint8_t scancode = 0;

int (write_command_KBC)(uint8_t port, uint8_t command) {
  uint8_t st;
  uint8_t iterations = 5;

  while (iterations--) {
    int ret = util_sys_inb(KBC_ST_REG, &st);
    if (ret != 0) {
      printf("Error reading from status register.\n");
      return 1;
    }

    if (!(st & INPUT_BUFFER_FULL)) {
      int ret = sys_outb(port, command);
      if (ret != 0) {
        printf("Error writing to buffer.\n");
        return 1;
      }
      return ret;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  return 1;
}

int (read_output_KBC)(uint8_t port, uint8_t *output, uint8_t mouse) {
  uint8_t st;
  uint8_t iterations = 5;

  while (iterations--) {
    int ret = util_sys_inb(KBC_ST_REG, &st); // always read before checking errors
    if (ret != 0) {
      printf("Error reading KBC status.\n");
      return 1;
    }

    if (st & OUTPUT_BUFFER) {
      ret = util_sys_inb(port, output);
      if (ret != 0) {
        printf("Error reading output.\n");
        return 1;
      }

      if ((st & (PARITY_STATUS | TIMEOUT_STATUS)) != 0) {
        printf("Parity or timeout error.\n");
        return 1;
      }

      if (mouse && !(st & AUX_MOUSE)) { 
        printf("Didn't receive output from mouse.\n");
        return 1;
      }

      if (!mouse && (st & AUX_MOUSE)) { 
        printf("Didn't receive output from keyboard.\n");
        return 1;
      }
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  return 1;
}

int (kbc_subscribe_int)(uint8_t *bit_no) {
  if (bit_no == NULL) {
    printf("Null pointer.\n");
    return 1;
  }
  *bit_no = (uint8_t) hook_id_kbc;

  int ret = sys_irqsetpolicy(IRQ_KEYBOARD, (IRQ_REENABLE | IRQ_EXCLUSIVE), &hook_id_kbc);
  if (ret != 0) {
    printf("Error subscribing keyboard.\n");
    return 1;
  }
  return 0;
}

int (kbc_unsubscribe_int)() {
  int ret = sys_irqrmpolicy(&hook_id_kbc);
  if (ret != 0) {
    printf("Error unsubscribing keyboard.\n");
    return 1;
  }
  return 0;
}

void (kbc_ih)() {
  int ret = read_output_KBC(OUT_BUFFER, &scancode, 0);
  if (ret != 0) {
    printf("Error reading scancode.\n");
  }
}
