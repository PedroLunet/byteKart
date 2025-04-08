#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "mouse.h"

uint8_t current_byte;
int hook_id_mouse = 3;

int (mouse_subscribe_int)(uint8_t *bit_no) {
  if (bit_no == NULL) {
    printf("Null pointer.");
    return 1;
  }
  *bit_no = (uint8_t) hook_id_mouse;

  int ret = sys_irqsetpolicy(IRQ_MOUSE, (IRQ_REENABLE | IRQ_EXCLUSIVE), &hook_id_mouse);
  if (ret != 0) {
    printf("Error subscribing mouse.");
    return 1;
  }
  return 0;
}

int (mouse_unsubscribe_int)() {
  int ret = sys_irqrmpolicy(&hook_id_mouse);
  if (ret != 0) {
    printf("Error unsubscribing mouse.");
    return 1;
  }
  return 0;
}

void (mouse_ih)() {
  if (read_output_KBC(WRITE_COMMAND_BYTE, &current_byte, 1)) {
    printf("Error reading byte from mouse.");
  }
}
