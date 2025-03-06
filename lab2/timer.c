#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int hook_id = 0;
int counter = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  uint8_t st;
  if (timer_get_conf(timer, &st) != 0) {
    printf("Error in timer_get_conf\n");
    return 1;
  }

  st &= 0x0F;

  switch(timer) {
    case 0:
      st |= TIMER_SEL0;
    break;
    case 1:
      st |= TIMER_SEL1;
    break;
    case 2:
      st |= TIMER_SEL2;
    break;
    default:
      printf("Invalid timer\n");
    return 1;
  }

  uint8_t control_word = st | TIMER_LSB_MSB;

  uint16_t initial_value = TIMER_FREQ / freq;
  uint8_t lsb, msb;

  util_get_LSB(initial_value, &lsb);
  util_get_MSB(initial_value, &msb);

  if (sys_outb(TIMER_CTRL, control_word) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }

  if (sys_outb(TIMER_0 + timer, lsb) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }

  if (sys_outb(TIMER_0 + timer, msb) != OK) {
    printf("Error in sys_outb\n");
    return 1;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = hook_id;
  if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != OK) {
    printf("Error in sys_irqsetpolicy\n");
    return 1;
  }

  return 0;
}

int (timer_unsubscribe_int)() {
    if(sys_irqrmpolicy(&hook_id) != OK) {
        printf("Error in sys_irqrmpolicy\n");
        return 1;
    }

  return 0;
}

void (timer_int_handler)() {
  counter++;
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
