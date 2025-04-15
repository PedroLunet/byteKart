#include <lcom/lcf.h>
#include <lcom/vbe.h>

#include "i8254.h"
#include "graphics.h"

uint8_t* frame_buffer;
vbe_mode_info_t mode_info;

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} rgb_8_8_8_t;

int(vg_set_mode)(uint16_t mode) {
  reg86_t reg86;
  memset(&reg86, 0, sizeof(reg86));

  reg86.intno = 0x10;
  reg86.ah = 0x4F;
  reg86.al = 0x02;
  reg86.bx = BIT(14) | mode;

  if (sys_int86(&reg86) != 0) {
    printf("Error in sys_int86\n");
    return 1;
  }

  return 0;
}

int(vg_set_frame_buffer)(uint16_t mode) {

  if (vbe_get_mode_info(mode, &mode_info) != 0) {
      printf("Error getting mode info\n");
      return 1;
  }

  unsigned int bytes_per_pixel = (mode_info.BitsPerPixel + 7) / 8;
  unsigned int frame_size = mode_info.XResolution * mode_info.YResolution * bytes_per_pixel;

  struct minix_mem_range physic_addresses;
  physic_addresses.mr_base = mode_info.PhysBasePtr;
  physic_addresses.mr_limit = physic_addresses.mr_base + frame_size;

  if (sys_privctl(SELF, SYS_PRIV_ADD_MEM, &physic_addresses)) {
    printf("Physical memory allocation error\n");
    return 1;
  }

  frame_buffer = vm_map_phys(SELF, (void*) physic_addresses.mr_base, frame_size);
  if (frame_buffer == NULL) {
    printf("Virtual memory allocation error\n");
    return 1;
  }

  return 0;
}

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  	// Iterate over each row of the rectangle
    for (unsigned i = 0; i < height; i++) {
      	if (vg_draw_hline(x, y+i, width, color) != 0) return 1;
    }
    return 0;
}

int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  for (unsigned i = 0 ; i < len ; i++)
    if (vg_draw_pixel(x+i, y, color) != 0) return 1;
  return 0;
}

int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {

  if(x > mode_info.XResolution || y > mode_info.YResolution) return 1;

  unsigned BytesPerPixel = (mode_info.BitsPerPixel + 7) / 8;

  unsigned int index = (mode_info.XResolution * y + x) * BytesPerPixel;

  if (memcpy(&frame_buffer[index], &color, BytesPerPixel) == NULL) return 1;

  return 0;
}

int (normalize_color)(uint32_t color, uint32_t *new_color) {
  if (mode_info.BitsPerPixel == 32) {
    *new_color = color;
  } else {
    *new_color = color & (BIT(mode_info.BitsPerPixel) - 1);
  }
  return 0;
}


