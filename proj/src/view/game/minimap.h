#ifndef MINIMAP_H
#define MINIMAP_H

#include "model/geometry.h"
#include "sprites/sprite.h"
#include "model/player.h"
#include "model/ai_car.h"
#include "road.h"

#include "view/utils/renderer.h"
#include "controller/video_card.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    Sprite* background_sprite;

    int screen_x;
    int screen_y;
    int display_width;
    int display_height;

    Point full_track_world_origin;
    float full_track_world_width;
    float full_track_world_height;

    float world_to_minimap_scale_x;
    float world_to_minimap_scale_y;

    uint32_t player_indicator_color;
    uint32_t ai_indicator_color;
    int indicator_size;

    float track_offset_x;
    float track_offset_y;

} Minimap;

int minimap_init(Minimap *minimap, xpm_map_t minimap_track_xpm, const Road *road_data, float track_offset_x, float track_offset_y);
void minimap_destroy(Minimap *minimap);
void minimap_draw(const Minimap *minimap, const Player *player, AICar *const ai_cars[], int num_ai_cars);

#endif //MINIMAP_H
