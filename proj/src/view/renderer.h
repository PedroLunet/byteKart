#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#include "controller/video_card.h"
#include "model/geometry.h"
#include "sprite.h"
#include "model/player.h"
#include "model/ai_car.h"
#include "view/road.h"

typedef struct {
  int x;
  int y;
} Point_i;

int renderer_init();
void renderer_clear_buffer(uint32_t color);
void renderer_transform_world_to_screen(const Player *player_view, Point world_pos, Point_i *screen_pos);
void renderer_draw_pixel(int x, int y, uint32_t color);
void renderer_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void renderer_draw_rectangle_filled(int x, int y, int width, int height, uint32_t color);

void renderer_draw_road(const Road *road, const Player *player_view);
void renderer_draw_player_car(const Player *player);
void renderer_draw_ai_car(const AICar *ai_car, const Player *player_view);

#endif //RENDERER_H
