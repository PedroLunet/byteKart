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

  if (st == NULL || timer < 0 || timer > 2) {
    printf("Input error in timer_get_conf()");
    return -1;
  }

  uint8_t readBackCommand = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));

  int ret; // return instruction
  ret = sys_outb(TIMER_CTRL, readBackCommand);
  if (ret != 0)
    return 1;
  
  ret = util_sys_inb(TIMER_0 + timer, st);
  if (ret != 0)
    return 1;

  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {

  union timer_status_field_val data;

  switch(field) {
    case tsf_all:
      data.byte = st;
      break;

    case tsf_initial:
      st = (st >> 4) & 0x03;

      if (st == 1) 
        data.in_mode = LSB_only;
      if (st == 2)
        data.in_mode = MSB_only;
      if (st == 3)
        data.in_mode = MSB_after_LSB;
      break;
    
    case tsf_mode:
      st = (st >> 1) & 0x07;
      if (st == 6) 
        data.count_mode = 2;
      if (st == 7)
        data.count_mode = 3;
      else
        data.count_mode = st;
      break;

    case tsf_base:
      data.bcd = st & TIMER_BCD;
      break; 
  }

  int ret = timer_print_config(timer, field, data);
  if (ret != 0)
    return 1;

  return 0;
}
