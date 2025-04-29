#include <lcom/lcf.h>
#include "kbc.h"

int syslib_count_lab3 = 0;

int (kbc_read_status)(uint8_t* status) {

  if (util_sys_inb(KBC_ST_REG, status) != 0) {
    return 1;
  }

  return 0;
}

int (kbc_write_command)(uint8_t port, uint8_t command) {

  uint8_t stat;
  uint8_t attempts = MAX_ATTEMPS;

  while (attempts) {

    if (kbc_read_status(&stat) != 0) {
      return 1;
    }

    #ifdef LAB3
        syslib_count_lab3++;
    #endif

    if ((stat & KBC_ST_IBF) == 0) {

      if (sys_outb(port, command) != 0) {
        return 1;
      }

      return 0;
    }
    tickdelay(micros_to_ticks(DELAY));
    attempts--;
  }

  return 1;
}

int (kbc_read_output)(uint8_t port, uint8_t *data, uint8_t mouse) {

  uint8_t stat;
  uint8_t attempts = MAX_ATTEMPS;

  while (attempts) {

    if (kbc_read_status(&stat) != 0) {
      return 1;
    }

    #ifdef LAB3
        syslib_count_lab3++;
    #endif

    if (stat & KBC_ST_OBF) {

      if (util_sys_inb(port, data) != 0) {
        return 1;
      }

      #ifdef LAB3
         syslib_count_lab3++;
      #endif

      if ((stat & (KBC_PAR_ERR | KBC_TO_ERR)) == 0) {
        if (stat & KBC_ST_AUX && mouse) {
          return 0;
        }
        else if (!(stat & KBC_ST_AUX) && !mouse) {
          return 0;
        }
        else {
          printf("Error: Output not from the expected device!\n");
          return 1;
        }
      }
      else {
        return 1;
      }
    }
    tickdelay(micros_to_ticks(DELAY));
    attempts--;
  }

  return 1;
}
