#include <lcom/lcf.h>
#include "player.h"

extern vbe_mode_info_t vbe_mode_info;

static float precalculated_cos_turn;
static float precalculated_sin_turn;
static bool player_module_turn_statics_initialized = false;

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

static void player_update_speed_and_velocity_mk(Player *player, bool skid_input, float delta_time) {
    float target_speed_now = player->base_speed * player->current_speed_modifier;

    if (skid_input) {
        player->current_speed -= player->skid_deceleration_value * delta_time;
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

int player_create(Player *player, Point initial_car_center_world, float initial_direction_rad, Road *road) {
    if (!player) return 1;
    if (!road) {
        fprintf(stderr, "player_create: Road pointer is NULL.\n");
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

    // Sprite *car_sprite = sprite_create_xpm((xpm_map_t)car_sprite_xpm, world_position_car_center.x, world_position_car_center.y, 0, 0);

    vector_init(&player->current_velocity, 0.0f, 0.0f);

    bool found_on_track = road_get_tangent_at_world_pos_FULLSCAN(road, &player->world_position_car_center,
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
        fprintf(stderr, "Warning: Player created off-track or on invalid track, defaulting to start.\n");
    }

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

    player_update_effects(player, delta_time);

    road_update_entity_on_track(road, &player->world_position_car_center,
                                &player->current_road_segment_idx,
                                &player->track_tangent_at_pos,
                                &player->closest_point_on_track);

    player_update_speed_and_velocity_mk(player, skid_input, delta_time);

    player_apply_world_movement(player, delta_time);
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
