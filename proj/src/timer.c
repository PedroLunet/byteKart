#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int timer_counter = 0;
int hook_id_timer;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  
  if (freq < 19 || freq > TIMER_FREQ) {
    return 1;
  } 

  uint8_t controlWord;
  if (timer_get_conf(timer, &controlWord) != 0) {
    return 1;
  }

  controlWord = (controlWord & 0x0F) | TIMER_LSB_MSB;

  uint8_t timer_sel;
  switch(timer) {
    case 0: 
      controlWord = controlWord | TIMER_SEL0;
      timer_sel = TIMER_0;
      break;
    case 1: 
      controlWord = controlWord | TIMER_SEL1;
      timer_sel = TIMER_1;
      break;
    case 2: 
      controlWord = controlWord | TIMER_SEL2;
      timer_sel = TIMER_2;
      break;
  }  

  uint32_t value = TIMER_FREQ / freq;
  uint8_t msb, lsb;
  util_get_MSB(value, &msb);
  util_get_LSB(value, &lsb);

  if (sys_outb(TIMER_CTRL, controlWord) != 0) {
    return 1;
  }

  // write MSB after LSB
  if (sys_outb(timer_sel, lsb) != 0) {
    return 1;
  }

  if (sys_outb(timer_sel, msb) != 0) {
    return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  if (bit_no == NULL) {
    return 1;
  }
  *bit_no = BIT(hook_id_timer);
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id_timer) != 0) {
    return 1;
  }
  return 0;
}

int (timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&hook_id_timer) != 0) {
    return 1;
  }
  return 0;
}

void (timer_int_handler)() {
  timer_counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {

  if (timer < 0 || timer > 2) {
    return 1;
  }

  if (st == NULL) {
    return -1;
  }

  uint8_t readBackCommand = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));

  if (sys_outb(TIMER_CTRL, readBackCommand) != 0) {
    return 1;
  }

  if (util_sys_inb(TIMER_0 + timer, st) != 0) {
    return 1;
  }

  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {

  union timer_status_field_val config;

  switch(field) {
    case tsf_all:
      config.byte = st;
      break;

    case tsf_initial:
      st = (st >> 4) & 0x03;
      if (st == 0)
        config.in_mode = INVAL_val;
      else if (st == 1) 
        config.in_mode = LSB_only;
      else if (st == 2)
        config.in_mode = MSB_only;
      else if (st == 3)
        config.in_mode = MSB_after_LSB;
      break;
    
    case tsf_mode:
      st = (st & TIMER_GET_MODE) >> 1;
      if (st == 6) 
        config.count_mode = 2;
      if (st == 7)
        config.count_mode = 3;
      else
        config.count_mode = st;
      break;

    case tsf_base:
      config.bcd = st & TIMER_GET_BASE;
      break; 
  }

  if (timer_print_config(timer, field, config) != 0) {
    return 1;
  }

  return 0;
}
