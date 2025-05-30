#include <lcom/lcf.h>
#include "ai_car.h"
#include "model/items.h"

extern int timer_counter;

static void ai_car_check_lap_completion(AICar *ai, Road *road) {
    if (!ai || !road || road->num_center_points < 2) return;

    int N_segments = road->num_center_points;

    if (ai->current_road_segment_idx != FINISH_LINE_SEGMENT_IDX &&
        ai->current_road_segment_idx != ((FINISH_LINE_SEGMENT_IDX - 1 + N_segments) % N_segments)) {
        ai->just_crossed_finish_this_frame = false;
    }

    if (ai->just_crossed_finish_this_frame) {
        return;
    }

    int prev_segment = ai->last_meaningful_road_segment_idx;
    int curr_segment = ai->current_road_segment_idx;

    int approach_zone_start_idx = (FINISH_LINE_SEGMENT_IDX + (int)((float)N_segments * LAP_APPROACH_ZONE_PERCENTAGE)) % N_segments;
    int departure_zone_end_idx  = (FINISH_LINE_SEGMENT_IDX + (int)((float)N_segments * LAP_DEPARTURE_ZONE_PERCENTAGE)) % N_segments;

    bool was_in_approach = (
        (approach_zone_start_idx <= FINISH_LINE_SEGMENT_IDX)
            ? (prev_segment >= approach_zone_start_idx && prev_segment < FINISH_LINE_SEGMENT_IDX)
            : (prev_segment >= approach_zone_start_idx || prev_segment < FINISH_LINE_SEGMENT_IDX)
    );

    bool is_in_departure = (
        (FINISH_LINE_SEGMENT_IDX <= departure_zone_end_idx)
            ? (curr_segment >= FINISH_LINE_SEGMENT_IDX && curr_segment < departure_zone_end_idx)
            : (curr_segment >= FINISH_LINE_SEGMENT_IDX || curr_segment < departure_zone_end_idx)
    );

    bool crossed_finish = (
        (prev_segment < FINISH_LINE_SEGMENT_IDX && curr_segment >= FINISH_LINE_SEGMENT_IDX) ||
        (prev_segment > curr_segment && (
            curr_segment >= FINISH_LINE_SEGMENT_IDX || prev_segment < FINISH_LINE_SEGMENT_IDX
        ))
    );

    if (was_in_approach && is_in_departure && crossed_finish) {
        if (ai->current_lap <= ai->total_laps) {
            ai->current_lap++;
            printf("AI Car %d on Lap: %d / %d\n", ai->id, (ai->current_lap > ai->total_laps ? ai->total_laps : ai->current_lap), ai->total_laps);
        }

        if (!ai->has_finished && ai->current_lap > ai->total_laps) {
            ai->has_finished = true;
            printf("AI Car %d has finished the race!\n", ai->id);
        }
        ai->just_crossed_finish_this_frame = true;
    }
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
    ai->last_stuck_check_position = start_pos;
    ai->stuck_eval_timer_s = AI_STUCK_EVAL_INTERVAL;


    vector_init(&ai->forward_direction, initial_direction.x, initial_direction.y);
    vector_normalize(&ai->forward_direction);
    vector_init(&ai->current_velocity, 0.0f, 0.0f);

    ai->difficulty = difficulty;
    ai->current_behavior_state = AI_STATE_RACING;
    ai->current_speed_modifier = 1.0f;
    ai->speed_modifier_duration_s = 0.0f;
    ai->state_timer_s = 0.0f;
    ai->time_continuously_stuck_s = 0.0f;

    // Set parameters based on difficulty
    switch (difficulty) {
        case AI_DIFFICULTY_EASY:
            ai->base_speed = AI_EASY_BASE_SPEED;
            ai->lookahead_distance = AI_EASY_LOOKAHEAD;
            ai->path_adherence_factor = AI_EASY_PATH_ADHERENCE;
            ai->unstick_aggressiveness = 0.5f;
            break;
        case AI_DIFFICULTY_MEDIUM:
            ai->base_speed = AI_MEDIUM_BASE_SPEED;
            ai->lookahead_distance = AI_MEDIUM_LOOKAHEAD;
            ai->path_adherence_factor = AI_MEDIUM_PATH_ADHERENCE;
            ai->unstick_aggressiveness = 1.0f;
            break;
        case AI_DIFFICULTY_HARD:
            ai->base_speed = AI_HARD_BASE_SPEED;
            ai->lookahead_distance = AI_HARD_LOOKAHEAD;
            ai->path_adherence_factor = AI_HARD_PATH_ADHERENCE;
            ai->unstick_aggressiveness = 1.5f;
            break;
        default: // Default to medium
            ai->base_speed = AI_MEDIUM_BASE_SPEED;
            ai->lookahead_distance = AI_MEDIUM_LOOKAHEAD;
            ai->path_adherence_factor = AI_MEDIUM_PATH_ADHERENCE;
            ai->unstick_aggressiveness = 1.0f;
            break;
    }
    ai->acceleration_value = AI_ACCELERATION;
    ai->deceleration_value = AI_DECELERATION;
    ai->max_steering_angle_rad = AI_MAX_STEERING_RATE_RAD_PER_SEC;

    bool found_on_track = road_get_tangent_at_world_pos_fullscan(road, &ai->world_position, &ai->track_tangent_at_pos, &ai->current_road_segment_idx, &ai->closest_point_on_track);

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
        printf("Warning: AI car %d created off-track, defaulting to start.\n", id);
    }
    ai->target_track_point = ai->closest_point_on_track;

    Sprite *car_sprite = sprite_create_xpm(car_sprite_xpm, start_pos.x, start_pos.y, 0, 0);
    ai->sprite = car_sprite;

    ai->current_lap = 0;
    ai->total_laps = MAX_LAPS;
    ai->just_crossed_finish_this_frame = false;
    ai->last_meaningful_road_segment_idx = (road->num_center_points > 1) ? (road->num_center_points - 2) : 0;
    ai->has_finished = false;
    ai->finish_time = 0.0f;

    ai->is_colliding_with_static_this_frame = false;

    ai->hitbox_half_width = car_sprite->width / 2.0f;
  	ai->hitbox_half_height = car_sprite->height / 2.0f;

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

void ai_car_update(AICar *ai, Road *road, Player *player, AICar *other_ai_cars[], int num_other_ai_cars, const GameItems *game_items, float delta_time) {
    if (!ai || !road || delta_time <= 0.0f) return;

    int N = road->num_center_points;
    int curr_idx = ai->current_road_segment_idx;
    int next_idx = (curr_idx + 1) % N;

    Point curr = road->center_points[curr_idx];
    Point next = road->center_points[next_idx];
    Point left = road->left_edge_points[curr_idx];
    Point right = road->right_edge_points[curr_idx];

    // Vetor tangente (direção da pista)
    float tx = next.x - curr.x;
    float ty = next.y - curr.y;
    float tlen = sqrtf(tx*tx + ty*ty);
    if (tlen < 1e-3f) return;
    tx /= tlen; ty /= tlen;

    float ahead_dist = 40.0f;

    float target_x, target_y;

    float lx = left.x, ly = left.y, rx = right.x, ry = right.y;
    float dx = rx - lx, dy = ry - ly;

    float seg_len2 = dx*dx + dy*dy;
    float t_atual = ((ai->world_position.x - lx) * dx + (ai->world_position.y - ly) * dy) / seg_len2;

    float t_alvo = 0.5f + ((rand() % 100) / 100.0f - 0.5f) * 0.7f;

    if (t_atual < 0.15f) t_alvo = 0.3f;
    else if (t_atual > 0.85f) t_alvo = 0.7f;

    if (tlen > 1e-3f) { tx /= tlen; ty /= tlen; }

    target_x = lx + t_alvo * dx + tx * ahead_dist;
    target_y = ly + t_alvo * dy + ty * ahead_dist;

    Vector to_target = { target_x - ai->world_position.x, target_y - ai->world_position.y, 0.0f };
    vector_init(&to_target, to_target.x, to_target.y);
    vector_normalize(&to_target);

    float steer = ai->forward_direction.x * to_target.y - ai->forward_direction.y * to_target.x;
    ai->current_steering_input = steer;
    if (ai->current_steering_input > 1.0f) ai->current_steering_input = 1.0f;
    if (ai->current_steering_input < -1.0f) ai->current_steering_input = -1.0f;

    float turn_amount_rad = ai->current_steering_input * ai->max_steering_angle_rad * delta_time;
    if (fabsf(turn_amount_rad) > 0.0001f) {
        float old_dx = ai->forward_direction.x, old_dy = ai->forward_direction.y;
        float cos_turn = cosf(turn_amount_rad), sin_turn = sinf(turn_amount_rad);
        ai->forward_direction.x = old_dx * cos_turn - old_dy * sin_turn;
        ai->forward_direction.y = old_dx * sin_turn + old_dy * cos_turn;
        vector_normalize(&ai->forward_direction);
    }

    ai->current_speed = ai->base_speed;

    ai->world_position.x += ai->forward_direction.x * ai->current_speed * delta_time;
    ai->world_position.y += ai->forward_direction.y * ai->current_speed * delta_time;

    obb_update(&ai->obb, ai->world_position, ai->forward_direction, ai->hitbox_half_width, ai->hitbox_half_height);
    road_update_entity_on_track(road, &ai->world_position, &ai->current_road_segment_idx, &ai->track_tangent_at_pos, &ai->closest_point_on_track);

    ai_car_check_lap_completion(ai, road);
    ai->last_meaningful_road_segment_idx = ai->current_road_segment_idx;
}

void ai_car_apply_speed_effect(AICar *ai, float modifier, float duration_s) {
    if (!ai) return;
    ai->current_speed_modifier = modifier;
    ai->speed_modifier_duration_s = duration_s;
}

void ai_car_handle_hard_collision(AICar *ai, float new_speed, bool hit_static_obstacle) {
    if (!ai) return;

    ai->current_speed = new_speed;
    ai->current_speed_modifier = 1.0f;
    ai->speed_modifier_duration_s = 0.0f;

    if (hit_static_obstacle) {
        ai->is_colliding_with_static_this_frame = true;
        if (fabsf(new_speed) < AI_STUCK_SPEED_THRESHOLD) {
            ai->time_continuously_stuck_s = AI_STUCK_TIME_THRESHOLD * 0.8f;
        }
    }

    ai->current_velocity.x = ai->current_speed * ai->forward_direction.x;
    ai->current_velocity.y = ai->current_speed * ai->forward_direction.y;
    vector_init(&ai->current_velocity, ai->current_velocity.x, ai->current_velocity.y);
}

void ai_car_set_finish_time(AICar *ai, float race_time) {
    if (!ai) return;
    if (ai->has_finished && ai->finish_time == 0.0f) {
        ai->finish_time = race_time;
    }
}

