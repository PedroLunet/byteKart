#include <lcom/lcf.h>
#include "video_card.h"

int (set_VBE_mode)(uint16_t mode) {
  reg86_t reg;
  memset(&reg, 0, sizeof(reg));

  reg.al = 0x02;
  reg.ah = 0x4F;
  reg.intno = 0x10;
  reg.bx = LINEAR_FRAME_BUF | mode;

  int ret = sys_int86(&reg);
  if (ret != 0) {
    printf("Error setting VBE mode.\n");
    return 1;
  }

  if (reg.al != 0x4F) {
    printf("Func not supported.\n");
    return 1;
  }

  if (reg.al == 0x4F) {
    printf("Func supported.\n");
    if (reg.ah == 0x01) {
      printf("Function call failed.\n");
      return 1;
    }
    else if (reg.ah == 0x02) {
      printf("Function not supported in current HW config.\n");
      return 1;
    }
    else if (reg.ah == 0x03) {
      printf("Function invalid in current video mode.\n");
      return 1;
    }
    else if (reg.ah == 0x00) {
      printf("Func successful.\n");
      return 0;
    }
  }

  return 1;
}
