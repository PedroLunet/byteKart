#ifndef AI_CAR_H
#define AI_CAR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "geometry.h"
#include "sprites/sprite.h"
#include "view/game/road.h"
#include "player.h"
#include "model/obb.h"
#include "model/physics_utils.h"

#include "lcom/timer.h"

struct GameItems_s;

typedef enum {
    AI_DIFFICULTY_EASY,
    AI_DIFFICULTY_MEDIUM,
    AI_DIFFICULTY_HARD
} AIDifficulty;

typedef enum {
    AI_STATE_RACING,
    AI_STATE_AVOIDING,
    AI_STATE_SEEKING_POWERUP,
    AI_STATE_UNSTICKING_REVERSING,
    AI_STATE_UNSTICKING_TURNING,
    AI_STATE_UNSTICKING_FORWARD_TRY,
    AI_STATE_FINISHED
} AIBehaviorState;

typedef struct AICar_s {
    int id;

    Point world_position;
    Vector forward_direction;
    float current_speed;
    Vector current_velocity;

    float base_speed;
    float current_speed_modifier;

    float acceleration_value;
    float deceleration_value;

    int current_road_segment_idx;
    Point closest_point_on_track;
    Vector track_tangent_at_pos;

    AIDifficulty difficulty;
    AIBehaviorState current_behavior_state;
    float state_timer_s;
    float time_continuously_stuck_s;
    Point last_stuck_check_position;
    float stuck_eval_timer_s;

    float lookahead_distance;
    Point target_track_point;
    float avoidance_steer_input;

    float max_steering_angle_rad;
    float current_steering_input;

    float path_adherence_factor;
    float unstick_aggressiveness;

    Sprite *sprite;

    float speed_modifier_duration_s;

    int current_lap;
    int total_laps;
    bool just_crossed_finish_this_frame;
    int last_meaningful_road_segment_idx;
    bool has_finished;
    float finish_time;

    bool is_colliding_with_static_this_frame;

    OBB obb;
	float hitbox_half_width;
  	float hitbox_half_height;

} AICar;

AICar* ai_car_create(int id, Point start_pos, Vector initial_direction, AIDifficulty difficulty, const char *const *car_sprite_xpm, Road *road);
void ai_car_destroy(AICar *this);
void ai_car_update(AICar *this, Road *road, Player *player, AICar *other_ai_cars[], int num_other_ai_cars, const struct GameItems_s *game_items, float delta_time);
void ai_car_apply_speed_effect(AICar *ai, float modifier, float duration_s);
void ai_car_handle_hard_collision(AICar *ai, float new_speed, bool hit_static_obstacle);
void ai_car_set_finish_time(AICar *ai, float race_time);

#endif //AI_CAR_H
