#ifndef _VIDEO_CARD_H_
#define _VIDEO_CARD_H_

#include <stdint.h>
#include <stdio.h>

#include "video_card_macros.h"

int (change_VBE_mode)(uint16_t mode);
int (start_VBE_mode)(uint16_t mode);
int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);
int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
int (vg_draw_matrix)(uint8_t no_rectangles, uint32_t first, uint8_t step);
int (vg_draw_xpm)(uint8_t *map, xpm_image_t *img, uint16_t x, uint16_t y);
int (vg_draw_rounded_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, uint32_t color);

uint16_t get_hres();
uint16_t get_vres();
uint8_t get_bits_per_pixel();

#endif
