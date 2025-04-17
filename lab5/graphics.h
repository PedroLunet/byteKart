#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <lcom/lcf.h>
#include "vbe.h"

vbe_mode_info_t mode_info;
uint8_t* frame_buffer;
uint8_t* back_buffer;

int (vg_set_mode)(uint16_t mode);
int (vg_set_frame_buffer)(uint16_t mode);
int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);
int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
int (normalize_color)(uint32_t color, uint32_t *new_color);
uint32_t (calculate_color)(uint16_t col, uint16_t row, uint8_t step, uint32_t first, uint8_t n, uint8_t mode);
int (print_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y);
void swap_buffers();

#endif /* GRAPHICS_H */
