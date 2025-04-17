#include <lcom/lcf.h>
#include "graphics.h"
#include <math.h>

int (vg_set_mode)(uint16_t mode) {
    reg86_t reg86;
    memset(&reg86, 0, sizeof(reg86));

    reg86.intno = 0x10;
    reg86.ah = 0x4F;
    reg86.al = 0x02;

    reg86.bx = mode | BIT(14);
    if (sys_int86(&reg86) != 0) {
        return 1;
    }
    return 0;
}

int (vg_set_frame_buffer)(uint16_t mode){

  memset(&mode_info, 0, sizeof(mode_info));
  if (vbe_get_mode_info(mode, &mode_info) != 0) {
    return 1;
  }

  unsigned int bytes_per_pixel = (mode_info.BitsPerPixel + 7) / 8;
  unsigned int frame_size = mode_info.XResolution * mode_info.YResolution * bytes_per_pixel;

  struct minix_mem_range physic_addresses;
  physic_addresses.mr_base = mode_info.PhysBasePtr;
  physic_addresses.mr_limit = physic_addresses.mr_base + frame_size;

  if (sys_privctl(SELF, SYS_PRIV_ADD_MEM, &physic_addresses)) {
    return 1;
  }

  frame_buffer = vm_map_phys(SELF, (void*) physic_addresses.mr_base, frame_size);
  if (frame_buffer == NULL) {
    return 1;
  }

  back_buffer = malloc(frame_size);
  if (back_buffer == NULL) {
    return 1;
}

  return 0;
}

int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {
  if (x >= mode_info.XResolution || y >= mode_info.YResolution) {
    return 1;
}

  unsigned bytes_per_pixel = (mode_info.BitsPerPixel + 7) / 8;
  unsigned int index = (mode_info.XResolution * y + x) * bytes_per_pixel;

  memcpy(&back_buffer[index], &color, bytes_per_pixel);
  return 0;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  for (unsigned i = 0 ; i < len ; i++)
    if (vg_draw_pixel(x+i, y, color) != 0) {
      return 1;
    }
  return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  for(unsigned i = 0; i < height ; i++)
    if (vg_draw_hline(x, y+i, width, color) != 0) {
      vg_exit();
      return 1;
    }
  return 0;
}

int normalize_color(uint32_t color, uint32_t *new_color) {
  if (mode_info.BitsPerPixel != 32) {
    *new_color = color & (BIT(mode_info.BitsPerPixel) - 1);
  } else {
    *new_color = color;
  }
  return 0;
}

uint32_t calculate_color(uint16_t col, uint16_t row, uint8_t step, uint32_t first, uint8_t n, uint8_t mode) {

  if (mode == DIRECT_COLOR) {
    uint32_t R = (((1 << mode_info.RedMaskSize) - 1) & (first >> mode_info.RedFieldPosition) + col * step) % (1 << mode_info.RedMaskSize);
    uint32_t G = (((1 << mode_info.GreenMaskSize) - 1) & (first >> mode_info.GreenFieldPosition) + row * step) % (1 << mode_info.GreenMaskSize);
    uint32_t B = (((1 << mode_info.BlueMaskSize) - 1) & (first >> mode_info.BlueFieldPosition) + (row + col) * step) % (1 << mode_info.BlueMaskSize);
    return (R << mode_info.RedFieldPosition) | (G << mode_info.GreenFieldPosition) | (B << mode_info.BlueFieldPosition);
  } else {
    return (first + (row * n + col) * step) % (1 << mode_info.BitsPerPixel);
  }

  return 1;
}

int (print_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

  xpm_image_t image;
  uint8_t *colors = xpm_load(xpm, XPM_INDEXED, &image);

  for (int height = 0 ; height < image.height ; height++) {
    for (int width = 0 ; width < image.width ; width++) {
      if (vg_draw_pixel(x + width, y + height, *colors) != 0) {
        return 1;
      }
      colors++;
    }
  }
  return 0;
}

void swap_buffers() {
  memcpy(frame_buffer, back_buffer, mode_info.XResolution * mode_info.YResolution * ((mode_info.BitsPerPixel + 7) / 8));
}
