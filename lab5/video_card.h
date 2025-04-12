#ifndef _VIDEO_CARD_H_
#define _VIDEO_CARD_H_

#include <stdint.h>
#include <stdio.h>

#include "video_card_macros.h"

int (change_VBE_mode)(uint16_t mode);
int (start_VBE_mode)(uint16_t mode);
int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color);
int (vg_draw_line)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);

#endif
