#include <lcom/lcf.h>

#include <stdint.h>

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  
  if (lsb == NULL)
    return 1;
  
  *lsb = val & 0xFF;

  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  
  if (msb == NULL)
    return 1;

  *msb = (val >> 8) & 0xFF;

  return 0;
}

#ifdef LAB3
  uint32_t interrupts = 0;
#endif

int (util_sys_inb)(int port, uint8_t *value) {
  #ifdef LAB3
    interrupts++;
  #endif

  if (value == NULL)
    return 1;

  uint32_t val;
  int ret = sys_inb(port, &val);
  *value = 0xFF & val;
  
  if (ret != 0)
    return 1;
  
  return 0;
}
