#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
    /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

int (timer_unsubscribe_int)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

void (timer_int_handler)() {
  /* To be implemented by the students */
  printf("%s is not yet implemented!\n", __func__);
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  uint8_t read_back_command = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);
  printf("read_back_command: %x\n", read_back_command);

  if (sys_outb(TIMER_CTRL, read_back_command) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }

  uint8_t status;
  if (util_sys_inb(TIMER_0 + timer, &status) != OK) {
    printf("Error in sys_inb\n");
    return 1;
  }

  printf("status: %x\n", status);

  *st = (uint8_t) status;

  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  union timer_status_field_val conf;

  switch (field) {
    case tsf_all:
      printf("Status byte: 0x%02x\n", st);
      conf.byte = st;
    break;
    case tsf_initial:
      printf("Initialization mode: %s\n", (st & (BIT(4) | BIT(5))) == TIMER_LSB ? "LSB" :
      (st & (BIT(4) | BIT(5))) == TIMER_MSB ? "MSB" :
      (st & (BIT(4) | BIT(5))) == TIMER_LSB_MSB ? "LSB followed by MSB" : "Unknown");
      conf.in_mode = (st & (BIT(4) | BIT(5)));
    break;
    case tsf_mode:
      printf("Operating mode: %d\n", (st & (BIT(1) | BIT(2) | BIT(3))) >> 1);
      conf.count_mode = (st & (BIT(1) | BIT(2) | BIT(3)) >> 1);
    break;
    case tsf_base:
      printf("Counting mode: %s\n", (st & TIMER_BCD) ? "BCD" : "Binary");
      conf.bcd = (st & TIMER_BCD);
    break;
    default:
      printf("Invalid field\n");
    return 1;
  }

  timer_print_config(timer, field, conf);

  return 0;
}
