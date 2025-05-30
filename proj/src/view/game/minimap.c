#include <lcom/lcf.h>
#include "minimap.h"

int minimap_init(Minimap *minimap, xpm_map_t minimap_track_xpm, const Road *road_data, float track_offset_x, float track_offset_y) {
    if (!minimap || !minimap_track_xpm || !road_data || !road_data->prerendered_track_image) {
        printf("Minimap Error: Invalid arguments for initialization.\n");
        return 1;
    }

    memset(minimap, 0, sizeof(Minimap));

    minimap->background_sprite = sprite_create_xpm(minimap_track_xpm, 0, 0, 0, 0);
    if (!minimap->background_sprite) {
        printf("Minimap Error: Failed to load minimap background sprite.\n");
        return 1;
    }

    minimap->screen_x = get_hres() - minimap->background_sprite->width - 30;
    minimap->screen_y = 30;

    minimap->display_width = (road_data->prerendered_track_image->height) * 0.02f;
    minimap->display_height = (road_data->prerendered_track_image->width) * 0.02f;

    // Store full track's world origin and dimensions
    minimap->full_track_world_origin = road_data->world_origin_of_track_image;

    minimap->full_track_world_width = (float)road_data->prerendered_track_image->height + track_offset_y;
    minimap->full_track_world_height = (float)road_data->prerendered_track_image->width + track_offset_x;

    minimap->track_offset_x = track_offset_x;
    minimap->track_offset_y = track_offset_y;

    if (minimap->full_track_world_width == 0 || minimap->full_track_world_height == 0) {
        printf("Minimap Error: Full track world dimensions are zero.\n");
        sprite_destroy(minimap->background_sprite);
        minimap->background_sprite = NULL;
        return 1;
    }

    minimap->world_to_minimap_scale_x = 0.02f;
    minimap->world_to_minimap_scale_y =  0.02f;

    minimap->player_indicator_color = 0x00FF00;
    minimap->ai_indicator_color = 0xFF0000;
    minimap->indicator_size = 3;

    return 0;
}

void minimap_destroy(Minimap *minimap) {
    if (!minimap) return;
    if (minimap->background_sprite) {
        sprite_destroy(minimap->background_sprite);
        minimap->background_sprite = NULL;
    }
}

static void world_to_minimap_coords(const Minimap *minimap, Point world_pos, Point_i *minimap_pixel_pos) {
    if (!minimap || !minimap_pixel_pos) return;

    float relative_world_x = world_pos.x - ;
    float relative_world_y = world_pos.y;

    minimap_pixel_pos->x = (int)(relative_world_x * minimap->world_to_minimap_scale_x);
    minimap_pixel_pos->y = (int)(minimap->display_height + (relative_world_y * minimap->world_to_minimap_scale_y));
}

void minimap_draw(const Minimap *minimap, const Player *player, AICar *const ai_cars[], int num_ai_cars) {
    if (!minimap) return;

    if (minimap->background_sprite) {
        sprite_draw_xpm(minimap->background_sprite, minimap->screen_x, minimap->screen_y, true);
    }

    Point_i indicator_pos;

    if (player) {
        world_to_minimap_coords(minimap, player->world_position_car_center, &indicator_pos);
        renderer_draw_rectangle_filled(
            minimap->screen_x + indicator_pos.x - minimap->indicator_size / 2,
            minimap->screen_y + indicator_pos.y - minimap->indicator_size / 2,
            minimap->indicator_size,
            minimap->indicator_size,
            minimap->player_indicator_color
        );
    }

    for (int i = 0; i < num_ai_cars; ++i) {
        if (ai_cars[i]) {
            world_to_minimap_coords(minimap, ai_cars[i]->world_position, &indicator_pos);
            renderer_draw_rectangle_filled(
                minimap->screen_x + indicator_pos.x - minimap->indicator_size / 2,
                minimap->screen_y + indicator_pos.y - minimap->indicator_size / 2,
                minimap->indicator_size,
                minimap->indicator_size,
                minimap->ai_indicator_color
            );
        }
    }
}


