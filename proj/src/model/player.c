#include <lcom/lcf.h>
#include "player.h"

extern vbe_mode_info_t vbe_mode_info;

static float precalculated_cos_turn;
static float precalculated_sin_turn;
static bool player_module_turn_statics_initialized = false;

static void initialize_player_turn_statics(void);
static void player_perceive_track_autopilot(Player *player, Road *road);
static void player_decide_steering_autopilot(Player *player);
static void player_apply_autopilot_movement(Player *player, float delta_time);

static void initialize_player_turn_statics() {
    if (!player_module_turn_statics_initialized) {
        precalculated_cos_turn = cos(PLAYER_TURN_INCREMENT_RAD);
        precalculated_sin_turn = sin(PLAYER_TURN_INCREMENT_RAD);
        player_module_turn_statics_initialized = true;
    }
}

static void player_update_effects(Player *player, float delta_time) {
    if (player->speed_modifier_duration_s > 0) {
        player->speed_modifier_duration_s -= delta_time;
        if (player->speed_modifier_duration_s <= 0) {
            player->current_speed_modifier = 1.0f;
            player->speed_modifier_duration_s = 0.0f;
        }
    }
}

static void player_update_speed_and_velocity_mk(Player *player, bool skid_input, float delta_time, bool in_recovery) {
    float target_speed_now = player->base_speed * player->current_speed_modifier;

    if (skid_input && player->current_speed > 350.0f) {
        player->current_speed -= player->skid_deceleration_value * delta_time;
    } else if (in_recovery) {
      player->current_speed += player->deceleration_value * delta_time * 0.5f;
    } else {
        if (player->current_speed < target_speed_now) {
            player->current_speed += player->acceleration_value * delta_time;
            if (player->current_speed > target_speed_now) {
                player->current_speed = target_speed_now;
            }
        } else if (player->current_speed > target_speed_now) {
            player->current_speed -= player->deceleration_value * delta_time;
            if (player->current_speed < target_speed_now) {
                player->current_speed = target_speed_now;
            }
        }
    }

    if (player->current_speed < 0.0f) player->current_speed = 0.0f;

    player->current_velocity.x = player->current_speed * player->forward_direction.x;
    player->current_velocity.y = player->current_speed * player->forward_direction.y;
    vector_init(&player->current_velocity, player->current_velocity.x, player->current_velocity.y);
}

static void player_apply_world_movement(Player *player, float delta_time) {
    player->world_position_car_center.x += player->current_velocity.x * delta_time;
    player->world_position_car_center.y += player->current_velocity.y * delta_time;

    player->viewport_world_top_left.x = player->world_position_car_center.x - (player->view_width / 2.0f);
    player->viewport_world_top_left.y = player->world_position_car_center.y - (player->view_height / 2.0f);
}

static void player_check_lap_completion(Player *player, Road *road) {
    if (!player || !road || road->num_center_points < 2) {
        return;
    }

    int N_segments = road->num_center_points;

    int prev_segment_for_lap = player->last_meaningful_road_segment_idx;
    int current_segment = player->current_road_segment_idx;

    int segment_just_before_finish = (FINISH_LINE_SEGMENT_IDX - 1 + N_segments) % N_segments;
    if (current_segment != FINISH_LINE_SEGMENT_IDX && current_segment != segment_just_before_finish) {
        player->just_crossed_finish_this_frame = false;
    }

    if (player->just_crossed_finish_this_frame) {
        return;
    }

    int approach_zone_start_idx = (FINISH_LINE_SEGMENT_IDX + (int)((float)N_segments * LAP_APPROACH_ZONE_PERCENTAGE)) % N_segments;
    int departure_zone_end_idx  = (FINISH_LINE_SEGMENT_IDX + (int)((float)N_segments * LAP_DEPARTURE_ZONE_PERCENTAGE)) % N_segments;

    bool was_in_approach_zone = (
        (approach_zone_start_idx <= FINISH_LINE_SEGMENT_IDX)
            ? (prev_segment_for_lap >= approach_zone_start_idx && prev_segment_for_lap < FINISH_LINE_SEGMENT_IDX)
            : (prev_segment_for_lap >= approach_zone_start_idx || prev_segment_for_lap < FINISH_LINE_SEGMENT_IDX)
    );

    bool is_in_departure_zone = (
        (FINISH_LINE_SEGMENT_IDX <= departure_zone_end_idx)
            ? (current_segment >= FINISH_LINE_SEGMENT_IDX && current_segment < departure_zone_end_idx)
            : (current_segment >= FINISH_LINE_SEGMENT_IDX || current_segment < departure_zone_end_idx)
    );

    bool crossed_finish = (
        (prev_segment_for_lap < FINISH_LINE_SEGMENT_IDX && current_segment >= FINISH_LINE_SEGMENT_IDX) ||
        (prev_segment_for_lap > current_segment && (
            current_segment >= FINISH_LINE_SEGMENT_IDX || prev_segment_for_lap < FINISH_LINE_SEGMENT_IDX
        ))
    );

    if (was_in_approach_zone && is_in_departure_zone && crossed_finish) {
        if (player->current_lap <= player->total_laps) {
            player->current_lap++;

            int display_lap = player->current_lap;
            if (player->current_lap > player->total_laps) {
                display_lap = player->total_laps;
                printf("Player has finished the race! (Completed Lap %d/%d)\n", display_lap, player->total_laps);
            } else {
                printf("Player on Lap: %d / %d\n", display_lap, player->total_laps);
            }
        }
        player->just_crossed_finish_this_frame = true;
    }
}

int player_create(Player *player, Point initial_car_center_world, float initial_direction_rad, Road *road, xpm_map_t car_sprite_xpm) {
    if (!player) return 1;
    if (!road) {
        printf("player_create: Road pointer is NULL.\n");
        return 1;
    }

    initialize_player_turn_statics();
    memset(player, 0, sizeof(Player));

    player->view_width = vbe_mode_info.XResolution;
    player->view_height = vbe_mode_info.YResolution;

    player->world_position_car_center = initial_car_center_world;

    player->viewport_world_top_left.x = player->world_position_car_center.x - (player->view_width / 2.0f);
    player->viewport_world_top_left.y = player->world_position_car_center.y - (player->view_height / 2.0f);

    vector_init(&player->forward_direction, cos(initial_direction_rad), sin(initial_direction_rad));
    vector_normalize(&player->forward_direction);

    player->base_speed = PLAYER_BASE_SPEED;
    player->current_speed_modifier = 1.0f;
    player->speed_modifier_duration_s = 0.0f;
    player->acceleration_value = PLAYER_ACCELERATION;
    player->deceleration_value = PLAYER_DECELERATION;
    player->skid_deceleration_value = PLAYER_SKID_DECELERATION;
    player->current_speed = 0.0f;

    vector_init(&player->current_velocity, 0.0f, 0.0f);

    bool found_on_track = road_get_tangent_at_world_pos_fullscan(road, &player->world_position_car_center,
                                                                &player->track_tangent_at_pos,
                                                                &player->current_road_segment_idx,
                                                                &player->closest_point_on_track);
    if (!found_on_track) {
        player->current_road_segment_idx = 0;
        if (road->num_center_points > 0) {
            player->closest_point_on_track = road->center_points[0];
            if (road->num_center_points > 1) {
                player->track_tangent_at_pos.x = road->center_points[1].x - road->center_points[0].x;
                player->track_tangent_at_pos.y = road->center_points[1].y - road->center_points[0].y;
            } else {
                player->track_tangent_at_pos.x = 1.0f; player->track_tangent_at_pos.y = 0.0f;
            }
        } else {
             player->closest_point_on_track = initial_car_center_world;
             player->track_tangent_at_pos.x = 1.0f; player->track_tangent_at_pos.y = 0.0f;
        }
        vector_init(&player->track_tangent_at_pos, player->track_tangent_at_pos.x, player->track_tangent_at_pos.y);
        vector_normalize(&player->track_tangent_at_pos);
        printf("Warning: Player created off-track or on invalid track, defaulting to start.\n");
    }

    if (car_sprite_xpm) {
        player->sprite = sprite_create_xpm((xpm_map_t) car_sprite_xpm, initial_car_center_world.x, initial_car_center_world.y, 0, 0);
        if (!player->sprite) {
            printf("Warning: Failed to load player car sprite.\n");
            return 1;
        }
    }

    player->current_lap = 0;
    player->total_laps = MAX_LAPS;
    player->just_crossed_finish_this_frame = false;
    player->last_meaningful_road_segment_idx = (road->num_center_points > 1) ? (road->num_center_points - 2) : 0;

    player->hitbox_half_width = player->sprite->width / 2.0f;
    player->hitbox_half_height = player->sprite->height / 2.0f;

    player->recovery_timer_s = 0.0f;

    player->autopilot_enabled = false;
    player->lookahead_distance = 100.0f;
    player->current_steering_input = 0.0f;
    player->max_steering_angle_rad = 1.5f;
    player->path_adherence_factor = 2.0f;
    player->target_track_point = initial_car_center_world;

    return 0;
}

void player_destroy(Player *player) {
    if (!player) return;
    if (player->sprite) {
        sprite_destroy(player->sprite);
        player->sprite = NULL;
    }
}

void player_handle_turn_input(Player *player, int turn_direction_sign) {
    if (!player || turn_direction_sign == 0) return;

    initialize_player_turn_statics();

    float old_dx = player->forward_direction.x;
    float old_dy = player->forward_direction.y;

    float effective_sin = turn_direction_sign * precalculated_sin_turn;
    float effective_cos = precalculated_cos_turn;

    player->forward_direction.x = old_dx * effective_cos - old_dy * effective_sin;
    player->forward_direction.y = old_dx * effective_sin + old_dy * effective_cos;

    vector_normalize(&player->forward_direction);

    player->current_velocity.x = player->current_speed * player->forward_direction.x;
    player->current_velocity.y = player->current_speed * player->forward_direction.y;
    vector_init(&player->current_velocity, player->current_velocity.x, player->current_velocity.y);
}

void player_update(Player *player, Road *road, bool skid_input, float delta_time) {
    if (!player || !road || delta_time <= 0.0f) return;

    bool is_in_recovery = false;
    if (player->recovery_timer_s > 0.0f) {
        is_in_recovery = true;
        player->recovery_timer_s -= delta_time;
        if (player->recovery_timer_s < 0.0f) player->recovery_timer_s = 0.0f;
    }

    player_update_effects(player, delta_time);

    road_update_entity_on_track(road, &player->world_position_car_center, &player->current_road_segment_idx, &player->track_tangent_at_pos, &player->closest_point_on_track);

    if (player->autopilot_enabled) {
        player_perceive_track_autopilot(player, road);
        player_decide_steering_autopilot(player);
        player_apply_autopilot_movement(player, delta_time);
    }

    player_update_speed_and_velocity_mk(player, (is_in_recovery ? false : skid_input), delta_time, is_in_recovery);

    player_apply_world_movement(player, delta_time);

    obb_update(&player->obb, player->world_position_car_center, player->forward_direction, player->hitbox_half_width, player->hitbox_half_height);

    player_check_lap_completion(player, road);

    player->last_meaningful_road_segment_idx = player->current_road_segment_idx;
}

static void player_perceive_track_autopilot(Player *player, Road *road) {
    float remaining_lookahead = player->lookahead_distance;
    Point current_p_on_line = player->closest_point_on_track;
    int p1_idx = player->current_road_segment_idx;

    player->target_track_point = current_p_on_line;

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
            player->target_track_point = current_p_on_line;
            continue;
        }

        if (remaining_lookahead <= segment_vec.magnitude) {
            float ratio = remaining_lookahead / segment_vec.magnitude;
            player->target_track_point.x = p1_on_segment.x + segment_vec.x * ratio;
            player->target_track_point.y = p1_on_segment.y + segment_vec.y * ratio;
            return;
        } else {
            remaining_lookahead -= segment_vec.magnitude;
            player->target_track_point = p2_on_segment;
            p1_idx = p2_idx;
        }
    }
}

static void player_decide_steering_autopilot(Player *player) {
    Vector vec_to_target;
    vec_to_target.x = player->target_track_point.x - player->world_position_car_center.x;
    vec_to_target.y = player->target_track_point.y - player->world_position_car_center.y;
    vector_init(&vec_to_target, vec_to_target.x, vec_to_target.y);

    if (vec_to_target.magnitude < 0.01f) {
        player->current_steering_input = 0.0f;
        return;
    }
    vector_normalize(&vec_to_target);

    Vector F = player->forward_direction;
    Vector T_norm = vec_to_target;

    float steering_value = F.x * T_norm.y - F.y * T_norm.x;

    player->current_steering_input = steering_value * player->path_adherence_factor * 1.5f;

    if (player->current_steering_input > 1.0f) player->current_steering_input = 1.0f;
    if (player->current_steering_input < -1.0f) player->current_steering_input = -1.0f;
}

static void player_apply_autopilot_movement(Player *player, float delta_time) {
    float turn_amount_rad = player->current_steering_input * player->max_steering_angle_rad * delta_time;

    float old_dx = player->forward_direction.x;
    float old_dy = player->forward_direction.y;
    float cos_turn = cos(turn_amount_rad);
    float sin_turn = sin(turn_amount_rad);

    player->forward_direction.x = old_dx * cos_turn - old_dy * sin_turn;
    player->forward_direction.y = old_dx * sin_turn + old_dy * cos_turn;
    vector_normalize(&player->forward_direction);

    player->current_velocity.x = player->current_speed * player->forward_direction.x;
    player->current_velocity.y = player->current_speed * player->forward_direction.y;
    vector_init(&player->current_velocity, player->current_velocity.x, player->current_velocity.y);
}

void player_enable_autopilot(Player *player) {
    if (!player) return;
    player->autopilot_enabled = true;
    player->current_steering_input = 0.0f;
}

void player_disable_autopilot(Player *player) {
    if (!player) return;
    player->autopilot_enabled = false;
    player->current_steering_input = 0.0f;
}

void player_apply_speed_effect(Player *player, float modifier, float duration_s) {
    if (!player) return;
    player->current_speed_modifier = modifier;
    player->speed_modifier_duration_s = duration_s;
}

void player_handle_hard_collision(Player *player, float new_speed) {
    if (!player) return;
    player->current_speed = new_speed;
    player->current_speed_modifier = 1.0f;
    player->speed_modifier_duration_s = 0.0f;
}
