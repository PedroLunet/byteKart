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
  memcpy(pos, &color, bytes_per_pixel);

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
