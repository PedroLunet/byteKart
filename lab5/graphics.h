#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

int(vg_info)(uint16_t mode, vbe_mode_info_t *mode_info);
int(vg_set_mode)(uint16_t mode);
int(vg_set_frame_buffer)(uint16_t mode);
int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
int(vg_draw_line)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int(vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);
int(normalize_color)(uint32_t color, uint32_t *new_color);

#endif //GRAPHICS_H
