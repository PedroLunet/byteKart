#include <lcom/lcf.h>
#include "items.h"
#include "model/ai_car.h"

int items_init(GameItems *items, const Road* road) {
    if (!items) return 1;

    printf("Items: Initializing game items...\n");
    srand(time(NULL));

    Sprite *oil_spill_sprite = sprite_create_xpm((xpm_map_t) oil_spill_xpm, 0,0,0,0);
    Sprite *barrier_sprite = sprite_create_xpm((xpm_map_t) barrier_xpm, 0,0,0,0);
    Sprite *powerup_box_sprite = sprite_create_xpm((xpm_map_t) powerup_box_xpm, 0,0,0,0);
    if (!oil_spill_sprite || !barrier_sprite || !powerup_box_sprite) {
        printf("Items: Failed to load item sprites.\n");
        return 1;
    }

    printf("Items: Successfully loaded item sprites.\n");
    // --- Initialize Obstacles  ---
	items->num_obstacles = road->num_obstacles_from_file;
	for (int i = 0; i < items->num_obstacles; ++i) {
    	RawObstacleData *raw = &road->raw_obstacle_data[i];
    	int seg_idx = raw->centerline_segment_index;
    	if (seg_idx < 0 || seg_idx >= road->num_center_points - 1) continue;

    	Point a = road->center_points[seg_idx];
    	Point b = road->center_points[seg_idx + 1];
    	float dx = b.x - a.x;
    	float dy = b.y - a.y;
    	float len = sqrtf(dx*dx + dy*dy);
    	float nx = -dy / len;
    	float ny = dx / len;

    	float cx = a.x + 0.5f * dx;
    	float cy = a.y + 0.5f * dy;

    	items->obstacles[i].world_position.x = cx + raw->offset_from_center * nx;
    	items->obstacles[i].world_position.y = cy + raw->offset_from_center * ny;
    	items->obstacles[i].is_active = true;
    	items->obstacles[i].type = raw->type;
      	items->obstacles[i].sprite = (raw->type == OBSTACLE_TYPE_OIL_SLICK) ? oil_spill_sprite : barrier_sprite;
    	items->obstacles[i].hitbox_half_width = 0.5f * items->obstacles[i].sprite->width;
        items->obstacles[i].hitbox_half_height = 0.5f * items->obstacles[i].sprite->height;

        Vector forward_dir = { dx / len, dy / len, 0.0f };
        vector_init(&forward_dir, forward_dir.x, forward_dir.y);
		obb_update(&items->obstacles[i].obb, items->obstacles[i].world_position, forward_dir, items->obstacles[i].hitbox_half_width, items->obstacles[i].hitbox_half_height);
	}

     printf("Items: Initialized %d obstacles.\n", items->num_obstacles);
    // --- Initialize Power-up Boxes ---
    items->num_powerup_boxes = 0;
	for (int i = 0; i < road->num_powerup_lines_from_file; ++i) {
    	RawPowerUpLineData *raw = &road->raw_powerup_data[i];
    	int seg_idx = raw->centerline_segment_index;
    	if (seg_idx < 0 || seg_idx >= road->num_center_points - 1) continue;

    	for (int j = 0; j < raw->num_boxes_in_line; ++j) {
    		float t = (j + 1.0f) / (raw->num_boxes_in_line + 1.0f);

    		Point left = road->left_edge_points[seg_idx];
    		Point right = road->right_edge_points[seg_idx];

   		 	items->powerup_boxes[items->num_powerup_boxes].world_position.x = left.x + t * (right.x - left.x);
    		items->powerup_boxes[items->num_powerup_boxes].world_position.y = left.y + t * (right.y - left.y);
    		items->powerup_boxes[items->num_powerup_boxes].is_active = true;
    		items->powerup_boxes[items->num_powerup_boxes].sprite_active = powerup_box_sprite;
        	items->powerup_boxes[items->num_powerup_boxes].respawn_timer_s = 10.0f;
            items->powerup_boxes[items->num_powerup_boxes].hitbox_half_width = 0.5f * items->powerup_boxes[items->num_powerup_boxes].sprite_active->width;
            items->powerup_boxes[items->num_powerup_boxes].hitbox_half_height = 0.5f * items->powerup_boxes[items->num_powerup_boxes].sprite_active->height;
    		obb_update(&items->powerup_boxes[items->num_powerup_boxes].obb, items->powerup_boxes[items->num_powerup_boxes].world_position, (Vector){1.0f, 0.0f, 0.0f}, items->powerup_boxes[items->num_powerup_boxes].hitbox_half_width, items->powerup_boxes[items->num_powerup_boxes].hitbox_half_height);

        	items->num_powerup_boxes++;
      	}
	}

    printf("Items: Initialized %d obstacles and %d power-up boxes.\n", items->num_obstacles, items->num_powerup_boxes);
    return 0;
}

void items_destroy(GameItems *items) {
    if (!items) return;
    for (int i = 0; i < items->num_obstacles; ++i) {
        if (items->obstacles[i].sprite) {
            sprite_destroy(items->obstacles[i].sprite);
            items->obstacles[i].sprite = NULL;
        }
    }
    for (int i = 0; i < items->num_powerup_boxes; ++i) {
        if (items->powerup_boxes[i].sprite_active) {
            sprite_destroy(items->powerup_boxes[i].sprite_active);
            items->powerup_boxes[i].sprite_active = NULL;
        }
    }
}

void items_update(GameItems *items, Player *player, AICar *ai_cars[], int num_ai_cars, const Road *road, float delta_time) {
    if (!items) return;

    CollisionInfo collision_info;
    float restitution_obstacle = 0.2f;

    // --- Update Power-up Box Respawning ---
    for (int i = 0; i < items->num_powerup_boxes; ++i) {
        PowerUpBox *box = &items->powerup_boxes[i];
        if (!box->is_active) {
            box->respawn_timer_s -= delta_time;
            if (box->respawn_timer_s <= 0.0f) {
                box->is_active = true;
                printf("PowerUp Box %d respawned.\n", i);
            }
        }
    }

    // --- Player Collisions with Items ---
    if (player) {
        // Player vs Obstacles
        for (int i = 0; i < items->num_obstacles; ++i) {
            Obstacle *obs = &items->obstacles[i];
            if (obs->is_active) {
                obb_check_collision_obb_vs_obb(&player->obb, &obs->obb, &collision_info);
                if (collision_info.occurred) {
                    // printf("Player collided with Obstacle %d!\n", i);

                    Vector push_normal = {-collision_info.collision_normal.x, -collision_info.collision_normal.y, 0.0f};
                    vector_init(&push_normal, push_normal.x, push_normal.y);
                    physics_resolve_overlap(&player->world_position_car_center, &push_normal, collision_info.penetration_depth);
                    obb_update(&player->obb, player->world_position_car_center, player->forward_direction, player->hitbox_half_width, player->hitbox_half_height);
                    physics_apply_bounce(&player->current_velocity, &player->current_speed, &player->forward_direction, &collision_info.collision_normal, restitution_obstacle);
                    // player_handle_hard_collision(player, player->current_speed * 0.5f); // Simpler effect
                }
            }
        }
        // Player vs PowerUp Boxes
        for (int i = 0; i < items->num_powerup_boxes; ++i) {
            PowerUpBox *box = &items->powerup_boxes[i];
            if (box->is_active) {
                obb_check_collision_obb_vs_obb(&player->obb, &box->obb, &collision_info);
                if (collision_info.occurred) {
                    printf("Player collected PowerUp Box %d!\n", i);
                    box->is_active = false;
                    box->respawn_timer_s = 10.0f;

                    // TODO: Implement player rank calculation to pass to items_assign_random_powerup
                    int player_rank = 1;
                    int total_racers = 1 + num_ai_cars;
                    PowerUpType received_powerup = items_assign_random_powerup(true, player_rank, total_racers);

                    // TODO: Store and handle 'received_powerup' for the player
                    printf("Player received power-up type: %d\n", received_powerup);
                    if (received_powerup == POWERUP_TYPE_SPEED_BOOST) {
                        player_apply_speed_effect(player, 1.5f, 3.0f);
                    }
                }
            }
        }
    }

    // --- AI Cars Collisions with Items ---
    for (int i = 0; i < num_ai_cars; ++i) {
        AICar *ai = ai_cars[i];
        if (!ai) continue;

        // AI vs Obstacles
        for (int j = 0; j < items->num_obstacles; ++j) {
            Obstacle *obs = &items->obstacles[j];
            if (obs->is_active) {
                obb_check_collision_obb_vs_obb(&ai->obb, &obs->obb, &collision_info);
                if (collision_info.occurred) {
                    // printf("AI Car %d collided with Obstacle %d!\n", ai->id, j);

                    Vector push_normal = {-collision_info.collision_normal.x, -collision_info.collision_normal.y, 0.0f};
                    vector_init(&push_normal, push_normal.x, push_normal.y);
                    physics_resolve_overlap(&ai->world_position, &push_normal, collision_info.penetration_depth);
                    obb_update(&ai->obb, ai->world_position, ai->forward_direction, ai->hitbox_half_width, ai->hitbox_half_height);
                    physics_apply_bounce(&ai->current_velocity, &ai->current_speed, &ai->forward_direction, &collision_info.collision_normal, restitution_obstacle);
                }
            }
        }
        // AI vs PowerUp Boxes
        for (int j = 0; j < items->num_powerup_boxes; ++j) {
            PowerUpBox *box = &items->powerup_boxes[j];
            if (box->is_active) {
                obb_check_collision_obb_vs_obb(&ai->obb, &box->obb, &collision_info);
                if (collision_info.occurred) {
                    printf("AI Car %d collected PowerUp Box %d!\n", ai->id, j);
                    box->is_active = false;
                    box->respawn_timer_s = 10.0f;

                    int ai_rank = i + 2;
                    int total_racers = 1 + num_ai_cars;
                    PowerUpType received_powerup = items_assign_random_powerup(false, ai_rank, total_racers);
                    // TODO: AI needs logic to store and use power-ups
                    printf("AI Car %d received power-up type: %d\n", ai->id, received_powerup);
                     if (received_powerup == POWERUP_TYPE_SPEED_BOOST) {
                        ai_car_apply_speed_effect(ai, 1.3f, 2.0f);
                    }
                }
            }
        }
    }
}

void items_draw(const GameItems *items, const Player *player_view) {
    if (!items || !player_view) return;
    Point_i screen_pos;

    // Draw Obstacles
    for (int i = 0; i < items->num_obstacles; ++i) {
        const Obstacle *obs = &items->obstacles[i];
        if (obs->is_active && obs->sprite) {
            renderer_transform_world_to_screen(player_view, obs->world_position, &screen_pos);

            float perp_x = obs->obb.axis[1].x * player_view->forward_direction.x + obs->obb.axis[1].y * player_view->forward_direction.y;
    		float perp_y = -obs->obb.axis[1].x * player_view->forward_direction.y + obs->obb.axis[1].y * player_view->forward_direction.x;

            sprite_draw_rotated_around_local_pivot(
                (Sprite*)obs->sprite,
                screen_pos.x, screen_pos.y,
                obs->sprite->width / 2, obs->sprite->height / 2,
                perp_x, -perp_y,
                true
            );
        }
    }

    // Draw Power-up Boxes
    for (int i = 0; i < items->num_powerup_boxes; ++i) {
        const PowerUpBox *box = &items->powerup_boxes[i];
        if (box->is_active && box->sprite_active) {
            renderer_transform_world_to_screen(player_view, box->world_position, &screen_pos);

            sprite_draw_rotated_around_local_pivot(
                (Sprite*)box->sprite_active,
                screen_pos.x, screen_pos.y,
                box->sprite_active->width / 2, box->sprite_active->height / 2,
                1.0f, 0.0f,
                true
            );
        }
    }
}

PowerUpType items_assign_random_powerup(bool car_is_player, int car_rank, int total_racers) {
    float chance_of_boost = 0.5f;

    if (total_racers > 1) {
        float rank_ratio = (float)(car_rank -1) / (float)(total_racers -1);
        chance_of_boost += rank_ratio * 0.4f;
    }
    if (chance_of_boost > 0.9f) chance_of_boost = 0.9f;
    if (chance_of_boost < 0.1f) chance_of_boost = 0.1f;

    float random_val = (float)rand() / (float)RAND_MAX;

    if (random_val < chance_of_boost) {
        printf("Assigned SPEED_BOOST (Rank: %d/%d, Chance: %.2f, Rolled: %.2f)\n", car_rank, total_racers, chance_of_boost, random_val);
        return POWERUP_TYPE_SPEED_BOOST;
    } else {
        printf("Assigned SHIELD (Rank: %d/%d, Chance: %.2f, Rolled: %.2f)\n", car_rank, total_racers, 1.0f - chance_of_boost, random_val);
        return POWERUP_TYPE_SHIELD;
    }
}
