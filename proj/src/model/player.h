#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../controller/video_card.h"
#include "geometry.h"
#include "macros.h"
#include "sprite.h"
#include "view/road.h"

struct Road_s;

typedef struct Player_s {

    Point viewport_world_top_left;
    int view_width;
    int view_height;

    Point world_position_car_center;
    Vector forward_direction;
    float current_speed;
    Vector current_velocity;

    float base_speed;
    float current_speed_modifier;
    float speed_modifier_duration_s;
    float acceleration_value;
    float deceleration_value;
    float skid_deceleration_value;

    int current_road_segment_idx;
    Point closest_point_on_track;
    Vector track_tangent_at_pos;

    int current_lap;
    int total_laps;
    bool just_crossed_finish_this_frame;
    int last_meaningful_road_segment_idx;

    Sprite* sprite;

} Player;

int player_create(Player *player, Point initial_car_center_world, float initial_direction_rad, Road *road, xpm_map_t car_sprite_xpm);
void player_destroy(Player *player);
void player_update(Player *player, Road *road, bool skid_input, float delta_time);
void player_handle_turn_input(Player *player, int turn_direction_sign);
void player_apply_speed_effect(Player *player, float modifier, float duration_s);
void player_handle_hard_collision(Player *player, float new_speed);

#endif // PLAYER_H
