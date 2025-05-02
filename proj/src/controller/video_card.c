#include <lcom/lcf.h>
#include "video_card.h"

vbe_mode_info_t vbe_mode_info;
void *video_memory;
struct minix_mem_range mmr;
unsigned int vram_size;
unsigned int vram_base;

uint8_t pixelBytes;
static uint16_t hres; /* XResolution */
static uint16_t vres; /* YResolution */
static uint8_t bits_per_pixel;

int (change_VBE_mode)(uint16_t mode) {
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
      return 0;
    }
  }

  return 1;
}

int (start_VBE_mode)(uint16_t mode) {
  static int call_vbe_once = 0;

  if (!call_vbe_once) {
    memset(&vbe_mode_info, 0, sizeof(vbe_mode_info));
    int ret = vbe_get_mode_info(mode, &vbe_mode_info);
    if (ret != 0) {
      printf("Error in VBE mode info.\n");
      return 1;
    }

    hres = vbe_mode_info.XResolution;
    vres = vbe_mode_info.YResolution;
    bits_per_pixel = vbe_mode_info.BitsPerPixel;

    vram_base = vbe_mode_info.PhysBasePtr;
    pixelBytes = bits_per_pixel / 8;  
    if (bits_per_pixel % 8 != 0)
      pixelBytes++;
    vram_size = hres * vres * pixelBytes;

    call_vbe_once = 1;
  }

  mmr.mr_base = (phys_bytes) vram_base;
  mmr.mr_limit = mmr.mr_base + vram_size;

  int ret = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mmr);
  if (sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mmr) != 0) {
    printf("sys_privctl failed: %d\n", ret);
    panic("sys_privctl failed: %d\n", ret);
    return 1;
  }

  video_memory = vm_map_phys(SELF, (void*) mmr.mr_base, vram_size);
  if (video_memory == ((void*) -1)) {   
    printf("Mapping video memory failed.\n");
    panic("Mapping video memory failed.\n");
    return 1;
  }

  memset(video_memory, 0, vram_size);

  ret = change_VBE_mode(mode);
  if (ret != 0) {
    printf("Error changing mode.\n");
    return 1;
  }

  return 0;
}

// Drawing functions
int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {
  if (x >= get_hres() || y >= get_vres() || x < 0 || y < 0) {
    printf("Size error.\n");
    return 1;
  }

  uint8_t bytes_per_pixel = (get_bits_per_pixel() + 7) / 8;
  uint8_t* pos = (uint8_t*) video_memory; 
  pos += (get_hres() * y + x) * bytes_per_pixel;

  if (get_bits_per_pixel() == 8) { // Indexed mode
    *pos = (uint8_t) color;
  } else { // Direct color mode
    memcpy(pos, &color, bytes_per_pixel);
  }
  
  return 0;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  for (uint16_t i = 0; i < len; i++) {
    if (vg_draw_pixel(x + i, y, color) != 0) {
      printf("Error drawing pixel at (%u, %u).\n", x + i, y);
      return 1;
    }
  }
  return 0;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  for (uint16_t i = 0; i < height; i++) {
    if (vg_draw_hline(x, y + i, width, color) != 0) {
      printf("Error drawing line at y = %u.\n", y + i);
      return 1;
    }
  }
  return 0;
}

// Get Methods
uint16_t get_hres() {
  return hres;
}

uint16_t get_vres() {
  return vres;
}

uint8_t get_bits_per_pixel() {
  return bits_per_pixel;
}

// Draw Matrix
int (vg_draw_matrix)(uint8_t no_rectangles, uint32_t first, uint8_t step) {

  uint16_t rect_width = get_hres() / no_rectangles;
  uint16_t rect_height = get_vres() / no_rectangles;

  for (uint8_t row = 0; row < no_rectangles; row++) {
    for (uint8_t col = 0; col < no_rectangles; col++) {
      uint32_t color;

      if (get_bits_per_pixel() == 8) {
        color = (first + (row * no_rectangles + col) * step) % (1 << get_bits_per_pixel());
      } else {
        uint8_t red_mask_size = vbe_mode_info.RedMaskSize;
        uint8_t green_mask_size = vbe_mode_info.GreenMaskSize;
        uint8_t blue_mask_size = vbe_mode_info.BlueMaskSize;

        uint32_t red = ((first >> vbe_mode_info.RedFieldPosition) & 0xFF) + col * step;
        uint32_t green = ((first >> vbe_mode_info.GreenFieldPosition) & 0xFF) + row * step;
        uint32_t blue = ((first >> vbe_mode_info.BlueFieldPosition) & 0xFF) + (col + row) * step;

        red = (red % (1 << red_mask_size)) << vbe_mode_info.RedFieldPosition;
        green = (green % (1 << green_mask_size)) << vbe_mode_info.GreenFieldPosition;
        blue = (blue % (1 << blue_mask_size)) << vbe_mode_info.BlueFieldPosition;

        color = red | green | blue;
      }

      int ret = vg_draw_rectangle(col * rect_width, row * rect_height, rect_width, rect_height, color);
      if (ret != 0) {
        printf("Error drawing rectange at row %u and col %u.\n", row, col);
        return 1;
      }
    }
  }

  uint16_t y = no_rectangles * rect_height;
  uint16_t x = 0;
  if (vg_draw_rectangle(x, y, get_hres(), get_vres() - y, 0x000000) != 0) {
    printf("Error drawing black bottom stripe.\n");
    return 1;
  }

  x = no_rectangles * rect_width;
  y = 0;
  if (vg_draw_rectangle(x, y, get_hres() - x, get_vres(), 0x000000) != 0) {
    printf("Error drawing black right stripe.\n");
    return 1;
  }

  return 0;
}

// Show XPM
int (vg_draw_xpm)(uint8_t *map, xpm_image_t *img, uint16_t x, uint16_t y) {
  if (map == NULL || img == NULL) {
    printf("Invalid map or image.\n");
    return 1;
  }

  uint8_t transparent_color = xpm_transparency_color(img->type); // To clear previous image

  for (uint16_t yi = 0; yi < img->height; yi++) {
    for (uint16_t xi = 0; xi < img->width; xi++) {
      uint8_t color = map[yi * img->width + xi];
      // Draw pixel
      if (color != transparent_color) {
        if ((x + xi) >= get_hres() || (y + yi) >= get_vres()) { // Boundary check
          continue;
        }
        
        if (vg_draw_pixel(x + xi, y + yi, color) != 0) {
          printf("Error drawing pixel at (%u, %u).\n", x+xi, y+yi);
          return 1;
        }
      }
    }
  }
  return 0;
}

int (vg_draw_rounded_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, uint32_t color) {
  if (radius > width / 2 || radius > height / 2) {
    printf("Radius is too large for the rectangle.\n");
    return 1;
  }

  // Draw the main rectangle (excluding corners)
  if (vg_draw_rectangle(x + radius, y, width - 2 * radius, height, color) != 0) return 1;
  if (vg_draw_rectangle(x, y + radius, radius, height - 2 * radius, color) != 0) return 1;
  if (vg_draw_rectangle(x + width - radius, y + radius, radius, height - 2 * radius, color) != 0) return 1;

  // Approximate the rounded corners (more precise method)
  // Top-left corner
  for (int i = 0; i <= radius; i++) {
    for (int j = 0; j <= radius; j++) {
      if (i * i + j * j <= radius * radius) {
        if (vg_draw_pixel(x + radius - i, y + radius - j, color) != 0) return 1;
      }
    }
  }

  // Top-right corner
  for (int i = 0; i <= radius; i++) {
    for (int j = 0; j <= radius; j++) {
      if (i * i + j * j <= radius * radius) {
        if (vg_draw_pixel(x + width - radius + i, y + radius - j, color) != 0) return 1;
      }
    }
  }

  // Bottom-left corner
  for (int i = 0; i <= radius; i++) {
    for (int j = 0; j <= radius; j++) {
      if (i * i + j * j <= radius * radius) {
        if (vg_draw_pixel(x + radius - i, y + height - radius + j, color) != 0) return 1;
      }
    }
  }

  // Bottom-right corner
  for (int i = 0; i <= radius; i++) {
    for (int j = 0; j <= radius; j++) {
      if (i * i + j * j <= radius * radius) {
        if (vg_draw_pixel(x + width - radius + i, y + height - radius + j, color) != 0) return 1;
      }
    }
  }

  return 0;
}
