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
    Point current_p_on_line = ai->closest_point_on_track;
    int p1_idx = ai->current_road_segment_idx;

    ai->target_track_point = current_p_on_line;

    int iterations = 0;
    int max_iterations = road->num_center_points * 2;
    if (max_iterations == 0 && road->num_center_points == 1) max_iterations = 1;

    while (remaining_lookahead > 0.001f && iterations < max_iterations) {
        iterations++;

        Point p1_on_segment = (iterations == 1) ? current_p_on_line : road->center_points[p1_idx];

        int p2_idx = (p1_idx + 1) % road->num_center_points;
        Point p2_on_segment = road->center_points[p2_idx];

        Vector segment_vec;
        segment_vec.x = p2_on_segment.x - p1_on_segment.x;
        segment_vec.y = p2_on_segment.y - p1_on_segment.y;
        vector_init(&segment_vec, segment_vec.x, segment_vec.y);

        if (segment_vec.magnitude < 0.0001f) {
            if (p1_idx == p2_idx) {
                break;
            }
            p1_idx = p2_idx;
            current_p_on_line = road->center_points[p1_idx];
            ai->target_track_point = current_p_on_line;
            continue;
        }

        if (remaining_lookahead <= segment_vec.magnitude) {
            float ratio = remaining_lookahead / segment_vec.magnitude;
            ai->target_track_point.x = p1_on_segment.x + segment_vec.x * ratio;
            ai->target_track_point.y = p1_on_segment.y + segment_vec.y * ratio;
            return; // Target found
        } else {
            remaining_lookahead -= segment_vec.magnitude;
            ai->target_track_point = p2_on_segment;
            p1_idx = p2_idx;
        }
    }
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

static void ai_car_check_lap_completion(AICar *ai, Road *road) {
    if (!ai || !road || road->num_center_points < 2) return;

    int N_segments = road->num_center_points;

    if (ai->current_road_segment_idx != FINISH_LINE_SEGMENT_IDX && ai->current_road_segment_idx != ((FINISH_LINE_SEGMENT_IDX - 1 + N_segments) % N_segments) ) {
        ai->just_crossed_finish_this_frame = false;
    }

    if (ai->just_crossed_finish_this_frame) {
        return;
    }

    int prev_segment = ai->last_meaningful_road_segment_idx;
    int curr_segment = ai->current_road_segment_idx;

    int approach_zone_start_idx = (int)(N_segments * LAP_APPROACH_ZONE_PERCENTAGE);
    int departure_zone_end_idx = (int)(N_segments * LAP_DEPARTURE_ZONE_PERCENTAGE);

    bool was_in_approach = (prev_segment >= approach_zone_start_idx && prev_segment < N_segments);
    bool is_in_departure = (curr_segment >= FINISH_LINE_SEGMENT_IDX && curr_segment < departure_zone_end_idx);

    if (was_in_approach && is_in_departure && prev_segment > curr_segment) {
        if (ai->current_lap <= ai->total_laps) {
            ai->current_lap++;
            printf("AI Car %d on Lap: %d / %d\n", ai->id, (ai->current_lap > ai->total_laps ? ai->total_laps : ai->current_lap), ai->total_laps);
        }

        if (ai->current_lap > ai->total_laps) {
            printf("AI Car %d has finished the race!\n", ai->id);
            // Optionally change AI behavior (e.g., slow down, change state to AI_STATE_FINISHED_RACE)
            // ai->base_speed *= 0.5f;
            // ai->current_behavior_state = AI_STATE_RECOVERING; // Or a new AI_STATE_FINISHED
        }
        ai->just_crossed_finish_this_frame = true; // Mark this crossing event
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
    bool found_on_track = road_get_tangent_at_world_pos_fullscan(road, &ai->world_position,
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

    Sprite *car_sprite = sprite_create_xpm(car_sprite_xpm, start_pos.x, start_pos.y, 0, 0);
    ai->sprite = car_sprite;

    ai->current_lap = 0;
    ai->total_laps = MAX_LAPS;
    ai->just_crossed_finish_this_frame = false;
    ai->last_meaningful_road_segment_idx = (road->num_center_points > 1) ? (road->num_center_points - 2) : 0;

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

void ai_car_update(AICar *this, Road *road, Player *player, AICar *other_ai_cars[], int num_other_ai_cars, float delta_time) {
    if (!this || !road || delta_time <= 0) return;

    ai_car_update_effects(this, delta_time);

    road_update_entity_on_track(road, &this->world_position, &this->current_road_segment_idx, &this->track_tangent_at_pos, &this->closest_point_on_track);
    ai_car_perceive_track(this, road);

    ai_car_decide_steering(this);

    // 5. (Future) React to player or other AI cars (collision avoidance, overtaking logic)
    // This is where 'player', 'other_ai_cars', 'num_other_ai_cars' would be used.
    // For now, we ignore them for simplicity.

    ai_car_apply_movement(this, delta_time);

    ai_car_check_lap_completion(this, road);
    this->last_meaningful_road_segment_idx = this->current_road_segment_idx;
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

