#include <lcom/lcf.h>
#include "ai_car.h"

static void ai_car_update_effects(AICar *ai, float delta_time) {
    if (ai->speed_modifier_duration_s > 0) {
        ai->speed_modifier_duration_s -= delta_time;
        if (ai->speed_modifier_duration_s <= 0) {
            ai->current_speed_modifier = 1.0f;
            ai->speed_modifier_duration_s = 0.0f;
        }
    }
}

static void ai_car_perceive_track(AICar *ai, Road *road) {

    float remaining_lookahead = ai->lookahead_distance;
    ai->target_track_point = ai->closest_point_on_track;
    int current_idx = ai->current_road_segment_idx;

    for (int i = current_idx; i < road->num_center_points - 1; ++i) {
        Point p1 = (i == current_idx) ? ai->closest_point_on_track : road->center_points[i];
        Point p2 = road->center_points[i+1];

        Vector segment_vec;
        segment_vec.x = p2.x - p1.x;
        segment_vec.y = p2.y - p1.y;
        vector_init(&segment_vec, segment_vec.x, segment_vec.y);

        if (segment_vec.magnitude < 0.0001f) continue;

        if (remaining_lookahead <= segment_vec.magnitude) {
            float ratio = remaining_lookahead / segment_vec.magnitude;
            ai->target_track_point.x = p1.x + segment_vec.x * ratio;
            ai->target_track_point.y = p1.y + segment_vec.y * ratio;
            return;
        } else {
            remaining_lookahead -= segment_vec.magnitude;
            ai->target_track_point = p2;
        }
    }
}

static float shortest_angle_diff_rad(float angle1_rad, float angle2_rad) {
    float diff = angle2_rad - angle1_rad;
    while (diff > M_PI) diff -= 2.0f * M_PI;
    while (diff < -M_PI) diff += 2.0f * M_PI;
    return diff;
}

static void ai_car_decide_steering(AICar *ai) {
    Vector vec_to_target;
    vec_to_target.x = ai->target_track_point.x - ai->world_position.x;
    vec_to_target.y = ai->target_track_point.y - ai->world_position.y;
    vector_init(&vec_to_target, vec_to_target.x, vec_to_target.y);

    if (vec_to_target.magnitude < 0.01f) {
        ai->current_steering_input = 0.0f;
        return;
    }
    vector_normalize(&vec_to_target);

    Vector F = ai->forward_direction;
    Vector T_norm = vec_to_target;

    float steering_value = F.x * T_norm.y - F.y * T_norm.x;

    ai->current_steering_input = steering_value * ai->path_adherence_factor * 1.5f;

    if (ai->current_steering_input > 1.0f) ai->current_steering_input = 1.0f;
    if (ai->current_steering_input < -1.0f) ai->current_steering_input = -1.0f;
}

static void ai_car_apply_movement(AICar *ai, float delta_time) {
    float target_speed = ai->base_speed * ai->current_speed_modifier;
    if (ai->current_speed < target_speed) {
        ai->current_speed += ai->acceleration_value * delta_time;
        if (ai->current_speed > target_speed) {
            ai->current_speed = target_speed;
        }
    } else if (ai->current_speed > target_speed) {
        ai->current_speed -= ai->deceleration_value * delta_time;
        if (ai->current_speed < target_speed) {
            ai->current_speed = target_speed;
        }
    }
    if (ai->current_speed < 0) ai->current_speed = 0;

    float turn_amount_rad = ai->current_steering_input * ai->max_steering_angle_rad * delta_time;

    float old_dx = ai->forward_direction.x;
    float old_dy = ai->forward_direction.y;
    float cos_turn = cos(turn_amount_rad);
    float sin_turn = sin(turn_amount_rad);

    ai->forward_direction.x = old_dx * cos_turn - old_dy * sin_turn;
    ai->forward_direction.y = old_dx * sin_turn + old_dy * cos_turn;
    vector_normalize(&ai->forward_direction);

    ai->world_position.x += ai->forward_direction.x * ai->current_speed * delta_time;
    ai->world_position.y += ai->forward_direction.y * ai->current_speed * delta_time;
}

AICar* ai_car_create(int id, Point start_pos, Vector initial_direction, AIDifficulty difficulty, const char *const *car_sprite_xpm, Road *road) {
    if (!road) {
        fprintf(stderr, "ai_car_create: Road pointer is NULL.\n");
        return NULL;
    }

    AICar *ai = (AICar*) malloc(sizeof(AICar));
    if (!ai) {
        perror("ai_car_create: Failed to allocate memory for AICar");
        return NULL;
    }

    memset(ai, 0, sizeof(AICar));

    ai->id = id;
    ai->world_position = start_pos;

    // Ensure initial direction is normalized
    vector_init(&ai->forward_direction, initial_direction.x, initial_direction.y);
    vector_normalize(&ai->forward_direction);

    ai->current_speed = 0.0f;
    ai->difficulty = difficulty;
    ai->current_behavior_state = AI_STATE_RACING;
    ai->current_speed_modifier = 1.0f;
    ai->speed_modifier_duration_s = 0.0f;

    // Set parameters based on difficulty
    switch (difficulty) {
        case AI_DIFFICULTY_EASY:
            ai->base_speed = AI_EASY_BASE_SPEED;
            ai->lookahead_distance = AI_EASY_LOOKAHEAD;
            ai->path_adherence_factor = AI_EASY_PATH_ADHERENCE;
            break;
        case AI_DIFFICULTY_MEDIUM:
            ai->base_speed = AI_MEDIUM_BASE_SPEED;
            ai->lookahead_distance = AI_MEDIUM_LOOKAHEAD;
            ai->path_adherence_factor = AI_MEDIUM_PATH_ADHERENCE;
            break;
        case AI_DIFFICULTY_HARD:
            ai->base_speed = AI_HARD_BASE_SPEED;
            ai->lookahead_distance = AI_HARD_LOOKAHEAD;
            ai->path_adherence_factor = AI_HARD_PATH_ADHERENCE;
            break;
        default: // Default to medium
            ai->base_speed = AI_MEDIUM_BASE_SPEED;
            ai->lookahead_distance = AI_MEDIUM_LOOKAHEAD;
            ai->path_adherence_factor = AI_MEDIUM_PATH_ADHERENCE;
            break;
    }
    ai->acceleration_value = AI_ACCELERATION;
    ai->deceleration_value = AI_DECELERATION;
    ai->max_steering_angle_rad = AI_MAX_STEERING_RATE_RAD_PER_SEC;

    // Initialize track-related info
    bool found_on_track = road_get_tangent_at_world_pos_FULLSCAN(road, &ai->world_position,
                                                                &ai->track_tangent_at_pos,
                                                                &ai->current_road_segment_idx,
                                                                &ai->closest_point_on_track);
    if (!found_on_track) {
        ai->current_road_segment_idx = 0;
        ai->closest_point_on_track = road->center_points[0];
        if (road->num_center_points > 1) {
            ai->track_tangent_at_pos.x = road->center_points[1].x - road->center_points[0].x;
            ai->track_tangent_at_pos.y = road->center_points[1].y - road->center_points[0].y;
        } else {
            ai->track_tangent_at_pos.x = 1.0f; ai->track_tangent_at_pos.y = 0.0f; // Default tangent
        }
        vector_init(&ai->track_tangent_at_pos, ai->track_tangent_at_pos.x, ai->track_tangent_at_pos.y);
        vector_normalize(&ai->track_tangent_at_pos);
        fprintf(stderr, "Warning: AI car %d created off-track, defaulting to start.\n", id);
    }
    ai->target_track_point = ai->closest_point_on_track;


    if (car_sprite_xpm) {
        ai->sprite = sprite_create((xpm_map_t)car_sprite_xpm);
    } else {
        ai->sprite = NULL;
    }

    return ai;
}

void ai_car_destroy(AICar *ai) {
    if (!ai) return;
    if (ai->sprite) {
        sprite_destroy(ai->sprite);
        ai->sprite = NULL;
    }
    free(ai);
}

void ai_car_update(AICar *ai, Road *road, const Player *player, AICar *other_ai_cars[], int num_other_ai_cars, float delta_time) {
    if (!ai || !road || delta_time <= 0) return;

    ai_car_update_effects(ai, delta_time);

    road_update_entity_on_track(road, &ai->world_position, &ai->current_road_segment_idx,
                                &ai->track_tangent_at_pos, &ai->closest_point_on_track);
    ai_car_perceive_track(ai, road);

    ai_car_decide_steering(ai);

    // 5. (Future) React to player or other AI cars (collision avoidance, overtaking logic)
    // This is where 'player', 'other_ai_cars', 'num_other_ai_cars' would be used.
    // For now, we ignore them for simplicity.

    ai_car_apply_movement(ai, delta_time);
}


void ai_car_apply_speed_effect(AICar *ai, float modifier, float duration_s) {
    if (!ai) return;
    ai->current_speed_modifier = modifier;
    ai->speed_modifier_duration_s = duration_s;
}

void ai_car_handle_hard_collision(AICar *ai, float new_speed) {
    if (!ai) return;
    ai->current_speed = new_speed;
    ai->current_speed_modifier = 1.0f;
    ai->speed_modifier_duration_s = 0.0f;
}