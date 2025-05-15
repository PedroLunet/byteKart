#ifndef AI_CAR_H
#define AI_CAR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "geometry.h"
#include "sprite.h"
#include "road.h"
#include "player.h"

typedef enum {
    AI_DIFFICULTY_EASY,
    AI_DIFFICULTY_MEDIUM,
    AI_DIFFICULTY_HARD
} AIDifficulty;

typedef enum {
    AI_STATE_RACING,
    AI_STATE_RECOVERING,
    // AI_STATE_OVERTAKING,
    // AI_STATE_AVOIDING_OBSTACLE
} AIBehaviorState;

typedef struct {
    int id;

    Point world_position;
    Vector forward_direction;
    float current_speed;

    float base_speed;
    float current_speed_modifier;

    float acceleration_value;
    float deceleration_value;

    int current_road_segment_idx;
    Point closest_point_on_track;
    Vector track_tangent_at_pos;

    AIDifficulty difficulty;
    AIBehaviorState current_behavior_state;

    float lookahead_distance;
    Point target_track_point;

    float max_steering_angle_rad;
    float current_steering_input;

    float path_adherence_factor;
    float obstacle_avoidance_skill;

    Sprite *sprite;

    float speed_modifier_duration_s;

} AICar;

AICar* ai_car_create(int id, Point start_pos, Vector initial_direction, AIDifficulty difficulty, const char *const *car_sprite_xpm, Road *road);
void ai_car_destroy(AICar *this);
void ai_car_update(AICar *this, Road *road, Player *player, AICar other_ai_cars[], int num_other_ai_cars, float delta_time);
void ai_car_apply_speed_effect(AICar *ai, float modifier, float duration_s);
void ai_car_handle_hard_collision(AICar *ai, float new_speed);

#endif //AI_CAR_H
