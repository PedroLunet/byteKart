#include <lcom/lcf.h>

#include "game.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern int timer_counter;
static UIComponent *countdownTextComponent = NULL;
extern Font *gameFont;
extern const xpm_map_t car_choices[6];

static void playing_draw_internal(GameState *base) {
    Game *this = (Game *)base;

    // Clear the screen
    renderer_clear_buffer(this->road_data.background_color);

    // Draw game elements
    if (this->current_running_state == GAME_SUBSTATE_PLAYING ||
        this->current_running_state == GAME_SUBSTATE_COUNTDOWN ||
        this->current_running_state == GAME_SUBSTATE_PAUSED ||
        this->current_running_state == GAME_SUBSTATE_FINISHED_RACE) {

        renderer_draw_road(&this->road_data, &this->player);

        for (int i = 0; i < this->num_active_ai_cars; ++i) {
            if (this->ai_cars[i]) {
                renderer_draw_ai_car(this->ai_cars[i], &this->player);
            }
        }
        renderer_draw_player_car(&this->player, this->player_skid_input_active, this->player_skid_input_sign, this->precomputed_cos_skid, this->precomputed_sin_skid);

        items_draw(&this->game_items, &this->player);

        // TODO: Draw HUD (laps, speed, timer)
        minimap_draw(&this->minimap, &this->player, this->ai_cars, this->num_active_ai_cars);
    }

    if (this->current_running_state == GAME_SUBSTATE_LOADING) { /* Draw loading screen */ return; }

    if (this->current_running_state == GAME_SUBSTATE_COUNTDOWN) {
        int count = this->timer_count_down;
        if (count > 0.0f && count <= 3.0f) {
          if (count % 1 == 0) {
             printf("Countdown: %d\n", (int)count);
          }
        } else if (count <= 0) {
            printf("GO!\n");
        }
    }

    if (this->current_running_state == GAME_SUBSTATE_PAUSED) {
        // Not drawing pause menu yet
    }
    else if (this->current_running_state == GAME_SUBSTATE_FINISHED_RACE) { /* Draw Race Finished UI */ }
}

static void playing_process_event_internal(GameState *base, EventType event) {
    Game *this = (Game *)base;
    if (!this) return;

    if (event == EVENT_TIMER) {
        this->base.update_state(base);
        this->base.draw(base);
    } else if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case LEFT_ARROW:
              this->player_turn_input_sign = 1;
              if (this->playerCar.x > 0) {
                car_move_left(&this->playerCar, LEFT_BOUNDARY, STEP);
              }
            break;
            case LEFT_ARROW_BREAK:
              if (this->player_turn_input_sign == 1) {
                this->player_turn_input_sign = 0;
              }
            break;
            case RIGHT_ARROW:
              this->player_turn_input_sign = -1;
              if (this->playerCar.x > 0) {
                 car_move_right(&this->playerCar, RIGHT_BOUNDARY, STEP);
              }
            break;
            case RIGHT_ARROW_BREAK:
              if (this->player_turn_input_sign == -1) {
                this->player_turn_input_sign = 0;
              }
            break;
            case SPACEBAR:
              this->player_skid_input_active = true;
              this->player_skid_input_sign = (this->player_turn_input_sign == 0) ? 0 : this->player_turn_input_sign;
            break;
            case SPACEBAR_BREAK:
              this->player_skid_input_active = false;
              this->player_skid_input_sign = 0;
            break;
            case P_KEY:
              if (this->current_running_state == GAME_SUBSTATE_PLAYING) {
                  this->current_running_state = GAME_SUBSTATE_PAUSED;
                  this->pause_requested = true;
              }
              if (this->current_running_state == GAME_SUBSTATE_PAUSED) {
                  this->current_running_state = GAME_SUBSTATE_PLAYING;
                  this->pause_requested = false;
              }
            break;
            case ESC_BREAKCODE:
                this->current_running_state = GAME_EXITED;
            break;
            default:
                break;
        }
    } else if (event == EVENT_MOUSE) {
        // Still missing pause menu and mouse handling for it
        if (this->current_running_state == GAME_SUBSTATE_PAUSED || this->current_running_state == GAME_SUBSTATE_FINISHED_RACE) {
          /*
            if (base->handle_mouse_input(base, (void (*)(GameState *))select_difficulty_clean_dirty_mouse_internal, select_difficulty_is_mouse_over, &this->selectedOption)) {
                this->chosenLevel = DIFFICULTY_SELECTED;
            }
           */
        }
        base->draw(base);
    }
}

static void update_countdown(Game *this, float delta_time) {
    if (this->timer_count_down > 0.0f) {
        this->timer_count_down -= delta_time;
        if (countdownTextComponent) {
            char countdown_str[5];
            int count = (int)this->timer_count_down;
            if (count > 0 && count <= 3) {
                sprintf(countdown_str, "%d", count);
            } else if (count <= 0) {
                sprintf(countdown_str, "GO!");
            } else {
                countdown_str[0] = '\0';
            }
            TextElementData *text_data = (TextElementData *)countdownTextComponent->data;
            if (text_data && strcmp(text_data->text, countdown_str) != 0) {
                free(text_data->text);
                text_data->text = strdup(countdown_str);
                free(text_data->pixel_data);
                text_data->width = 0;
                text_data->height = 0;
                if (text_data->text && text_data->font) {
                    int temp_x = 0;
                    int max_height = 0;
                    for (int i = 0; text_data->text[i] != '\0'; i++) {
                        GlyphData glyphData;
                        if (font_get_glyph_data(text_data->font, text_data->text[i], &glyphData)) {
                            temp_x += glyphData.xadvance;
                            if (glyphData.height > max_height) {
                                max_height = glyphData.height;
                            }
                        }
                    }
                    text_data->width = temp_x;
                    text_data->height = max_height;
                }
                if (load_text(text_data->text, 0, 0, text_data->color, text_data->font, text_data->pixel_data, text_data->width) != 0) {
                    printf("Error loading countdown text: %s\n", text_data->text);
                }
                countdownTextComponent->x = vbe_mode_info.XResolution / 2 - text_data->width / 2;
                countdownTextComponent->y = vbe_mode_info.YResolution / 2 - text_data->height / 2;
            }
        }
    }
    if (this->timer_count_down <= 0) {
        this->current_running_state = GAME_SUBSTATE_PLAYING;
        if (countdownTextComponent) {
            destroy_ui_component(countdownTextComponent);
            countdownTextComponent = NULL;
        }
    }
}

static void playing_update_internal(GameState *base) {
    Game *this = (Game *)base;
    if (!this) return;

    float delta_time = 1.0f / 60.0f;

    if (this->current_running_state == GAME_SUBSTATE_PLAYING) {
         obb_update(&this->player.obb, this->player.world_position_car_center, this->player.forward_direction,
                    this->player.hitbox_half_width, this->player.hitbox_half_height);
        for (int i = 0; i < this->num_active_ai_cars; ++i) {
            if (this->ai_cars[i]) {
                obb_update(&this->ai_cars[i]->obb, this->ai_cars[i]->world_position, this->ai_cars[i]->forward_direction, this->ai_cars[i]->hitbox_half_width, this->ai_cars[i]->hitbox_half_height);
            }
        }
    }

    switch(this->current_running_state) {
        case GAME_SUBSTATE_LOADING:
            // if (assets_loaded()) this->current_running_state = GAME_RUN_STATE_COUNTDOWN;
            break;
        case GAME_SUBSTATE_COUNTDOWN:
            update_countdown(this, delta_time);
            break;
        case GAME_SUBSTATE_PLAYING:
          	this->current_lap = this->player.current_lap;

            player_handle_turn_input(&this->player, this->player_turn_input_sign);
            player_update(&this->player, &this->road_data, this->player_skid_input_active, delta_time);
            obb_update(&this->player.obb, this->player.world_position_car_center, this->player.forward_direction, this->player.hitbox_half_width, this->player.hitbox_half_height);

            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i]) {
                    ai_car_update(this->ai_cars[i], &this->road_data, &this->player, NULL, 0, delta_time);
                    obb_update(&this->ai_cars[i]->obb, this->ai_cars[i]->world_position, this->ai_cars[i]->forward_direction, this->ai_cars[i]->hitbox_half_width, this->ai_cars[i]->hitbox_half_height);
                }
            }

            items_update(&this->game_items, &this->player, this->ai_cars, this->num_active_ai_cars, &this->road_data, delta_time);
            obb_update(&this->player.obb, this->player.world_position_car_center, this->player.forward_direction, this->player.hitbox_half_width, this->player.hitbox_half_height);
            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i]) {
                    obb_update(&this->ai_cars[i]->obb, this->ai_cars[i]->world_position, this->ai_cars[i]->forward_direction, this->ai_cars[i]->hitbox_half_width, this->ai_cars[i]->hitbox_half_height);
                }
            }

            CollisionInfo collision_info;
    		float restitution = 0.8f;

        	// Player vs. AI Cars
            for (int i = 0; i < this->num_active_ai_cars; ++i) {
        		if (this->ai_cars[i]) {
            		obb_check_collision_obb_vs_obb(&this->player.obb, &this->ai_cars[i]->obb, &collision_info);
            		if (collision_info.occurred) {
                		// printf("Collision: Player vs AI %d (Depth: %f, Normal: %f,%f)\n", this->ai_cars[i]->id, collision_info.penetration_depth, collision_info.collision_normal.x, collision_info.collision_normal.y);

                		Vector player_mtv_normal = {-collision_info.collision_normal.x, -collision_info.collision_normal.y, 0.0f};
                        vector_init(&player_mtv_normal, player_mtv_normal.x, player_mtv_normal.y);
                		physics_resolve_overlap(&this->player.world_position_car_center, &player_mtv_normal, collision_info.penetration_depth * 0.5f);

                		Vector ai_mtv_normal = collision_info.collision_normal;
                		physics_resolve_overlap(&this->ai_cars[i]->world_position, &ai_mtv_normal, collision_info.penetration_depth * 0.5f);


                		obb_update(&this->player.obb, this->player.world_position_car_center, this->player.forward_direction, this->player.hitbox_half_width, this->player.hitbox_half_height);
                		obb_update(&this->ai_cars[i]->obb, this->ai_cars[i]->world_position, this->ai_cars[i]->forward_direction, this->ai_cars[i]->hitbox_half_width, this->ai_cars[i]->hitbox_half_height);

                		physics_apply_bounce(&this->player.current_velocity, &this->player.current_speed, &this->player.forward_direction, &collision_info.collision_normal, restitution);

                		Vector ai_surface_normal = {-collision_info.collision_normal.x, -collision_info.collision_normal.y, 0.0f};
                        vector_init(&ai_surface_normal, ai_surface_normal.x, ai_surface_normal.y);
                		physics_apply_bounce(&this->ai_cars[i]->current_velocity, &this->ai_cars[i]->current_speed, &this->ai_cars[i]->forward_direction, &ai_surface_normal, restitution);
            		}
        		}
    		}

    		// AI vs. AI Cars
    		for (int i = 0; i < this->num_active_ai_cars; ++i) {
        		if (!this->ai_cars[i]) continue;
        		for (int j = i + 1; j < this->num_active_ai_cars; ++j) {
            		if (!this->ai_cars[j]) continue;

            		obb_check_collision_obb_vs_obb(&this->ai_cars[i]->obb, &this->ai_cars[j]->obb, &collision_info);
            		if (collision_info.occurred) {
                		// printf("Collision: AI %d vs AI %d (Depth: %f)\n", this->ai_cars[i]->id, this->ai_cars[j]->id, collision_info.penetration_depth);

                		Vector ai1_mtv_normal = {-collision_info.collision_normal.x, -collision_info.collision_normal.y, 0.0f};
                        vector_init(&ai1_mtv_normal, ai1_mtv_normal.x, ai1_mtv_normal.y);
                		physics_resolve_overlap(&this->ai_cars[i]->world_position, &ai1_mtv_normal, collision_info.penetration_depth * 0.5f);

                		Vector ai2_mtv_normal = collision_info.collision_normal;
                		physics_resolve_overlap(&this->ai_cars[j]->world_position, &ai2_mtv_normal, collision_info.penetration_depth * 0.5f);

                		obb_update(&this->ai_cars[i]->obb, this->ai_cars[i]->world_position, this->ai_cars[i]->forward_direction, this->ai_cars[i]->hitbox_half_width, this->ai_cars[i]->hitbox_half_height);
                		obb_update(&this->ai_cars[j]->obb, this->ai_cars[j]->world_position, this->ai_cars[j]->forward_direction, this->ai_cars[j]->hitbox_half_width, this->ai_cars[j]->hitbox_half_height);

                		physics_apply_bounce(&this->ai_cars[i]->current_velocity, &this->ai_cars[i]->current_speed, &this->ai_cars[i]->forward_direction, &collision_info.collision_normal, restitution);
                		Vector ai2_surface_normal = {-collision_info.collision_normal.x, -collision_info.collision_normal.y, 0.0f};
                        vector_init(&ai2_surface_normal, ai2_surface_normal.x, ai2_surface_normal.y);
                		physics_apply_bounce(&this->ai_cars[j]->current_velocity, &this->ai_cars[j]->current_speed, &this->ai_cars[j]->forward_direction, &ai2_surface_normal, restitution);
            		}
        		}
    		}

        	int player_seg_idx = this->player.current_road_segment_idx;
            int search_radius_edges = 30;
            int N_road_points = this->road_data.num_center_points;
            float track_restitution = 0.8f;

            if (N_road_points >= 2 && this->road_data.left_edge_points && this->road_data.right_edge_points) {
                for (int k = 0; k < (2 * search_radius_edges + 1); ++k) {
                    int offset = k - search_radius_edges;
                    int seg_to_check = (player_seg_idx + offset % N_road_points + N_road_points) % N_road_points;
                    int next_seg_to_check = (seg_to_check + 1) % N_road_points;

                    Point p0_left = this->road_data.left_edge_points[seg_to_check];
                    Point p1_left = this->road_data.left_edge_points[next_seg_to_check];
                    obb_check_collision_obb_vs_line_segment(&this->player.obb, p0_left, p1_left, &collision_info);
                    if (collision_info.occurred) {
                        // printf("Collision: Player vs Left Edge (Seg %d, Depth: %f)\n", seg_to_check, collision_info.penetration_depth);

                        physics_apply_bounce(&this->player.current_velocity, &this->player.current_speed, &this->player.forward_direction, &collision_info.collision_normal, track_restitution);

                        Vector push_normal = {-collision_info.collision_normal.x, -collision_info.collision_normal.y, 0.0f};
                        vector_init(&push_normal, push_normal.x, push_normal.y);
                        float effective_depth = collision_info.penetration_depth;
                        if (effective_depth < 0.01f && effective_depth > -0.01f) effective_depth = 0.5f;
                        else if (effective_depth < 0) effective_depth = 0.5f;
                        else effective_depth += 0.1f;
                        physics_resolve_overlap(&this->player.world_position_car_center, &push_normal, effective_depth);

                        player_handle_hard_collision(&this->player, this->player.current_speed * 0.1f);

                        obb_update(&this->player.obb, this->player.world_position_car_center, this->player.forward_direction, this->player.hitbox_half_width, this->player.hitbox_half_height);

                        this->player.recovery_timer_s = 0.25f;
                        break;
                    }

                    Point p0_right = this->road_data.right_edge_points[seg_to_check];
                    Point p1_right = this->road_data.right_edge_points[next_seg_to_check];
                    obb_check_collision_obb_vs_line_segment(&this->player.obb, p0_right, p1_right, &collision_info);
                    if (collision_info.occurred) {
                        // printf("Collision: Player vs Right Edge (Seg %d, Depth: %f)\n", seg_to_check, collision_info.penetration_depth);

                        physics_apply_bounce(&this->player.current_velocity, &this->player.current_speed, &this->player.forward_direction, &collision_info.collision_normal, track_restitution);

                        Vector push_normal = {-collision_info.collision_normal.x, -collision_info.collision_normal.y, 0.0f};
                        vector_init(&push_normal, push_normal.x, push_normal.y);

                        float effective_depth = collision_info.penetration_depth;
                        if (effective_depth < 0.01f && effective_depth > -0.01f) effective_depth = 0.5f;
                        else if (effective_depth < 0) effective_depth = 0.5f;
                        else effective_depth += 0.1f;

                        physics_resolve_overlap(&this->player.world_position_car_center, &push_normal, effective_depth);
                        player_handle_hard_collision(&this->player, this->player.current_speed * 0.1f);

                        obb_update(&this->player.obb, this->player.world_position_car_center, this->player.forward_direction, this->player.hitbox_half_width, this->player.hitbox_half_height);
                        this->player.recovery_timer_s = 0.25f;
                        break;
                    }
                }
            }

            if (this->current_lap > this->total_laps) {
                this->current_running_state = GAME_SUBSTATE_FINISHED_RACE;
                printf("Player finished all laps!\n");
            }

            break;
        case GAME_SUBSTATE_PAUSED:
            break;
        case GAME_SUBSTATE_FINISHED_RACE:
            // Maybe some post-race animation or waiting for input
            break;
        case GAME_STATE_EXITING:
            // Cleanup and exit logic
            break;
        case GAME_EXITED:
            // Cleanup and exit logic
            break;
    }
}

static void playing_destroy_internal(GameState *base) {
    Game *this = (Game *)base;
    if (!this) return;

    sprite_destroy(this->playerCar.car_sprite);
    sprite_destroy(this->road_sprite1);
    sprite_destroy(this->road_sprite2);

    items_destroy(&this->game_items);
    road_destroy(&this->road_data);
    minimap_destroy(&this->minimap);
    player_destroy(&this->player);

    for (int i = 0; i < MAX_AI_CARS; i++) {
        ai_car_destroy(this->ai_cars[i]);
    }

    if (countdownTextComponent) {
        destroy_ui_component(countdownTextComponent);
        countdownTextComponent = NULL;
    }

    free(this);
    free(base);
}

Game *game_state_create_playing(int difficulty, int car_choice, char *road_data_file, char *road_surface_file, float track_offset_x, float track_offset_y, uint32_t road_bg_color, xpm_map_t road_map_xpm) {
    Game *this = (Game *) malloc(sizeof(Game));
    if (this == NULL) {
        return NULL;
    }
    memset(this, 0, sizeof(Game));

    // Initialize GameState
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))playing_draw_internal;
    this->base.process_event = playing_process_event_internal;
    this->base.update_state = playing_update_internal;
    this->base.destroy = playing_destroy_internal;

    this->current_running_state = GAME_SUBSTATE_LOADING;
  	LoadingUI *loading_ui = loading_ui_create(gameFont, vbe_mode_info.XResolution, vbe_mode_info.YResolution);
	draw_ui_component(loading_ui->components[0]);
    swap_buffer_loading_ui();

    // Initialize Renderer
    if (renderer_init() != 0) {
        printf("game_state_create_playing: Failed to initialize renderer.\n");
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    this->playerCar.x = vbe_mode_info.XResolution / 2 - 30;
    this->playerCar.y = vbe_mode_info.YResolution - 100;
    this->playerCar.speed = 5;
    this->precomputed_cos_skid = cos(PLAYER_SKID_ANGLE);
    this->precomputed_sin_skid = sin(PLAYER_SKID_ANGLE);

    this->playerCar.car_sprite = sprite_create_xpm(car_choices[car_choice], 0, 0, 0, 0);
    if (!this->playerCar.car_sprite) {
        free(this);
        return NULL;
    }

    this->road_sprite1 = sprite_create_xpm((xpm_map_t) road_xpm, 0, 0, 0, 0);
    this->road_sprite2 = sprite_create_xpm((xpm_map_t) road_xpm, 0, 0, 0, 0);
    if (!this->road_sprite1 || !this->road_sprite2) {
        sprite_destroy(this->playerCar.car_sprite);
        if (this->road_sprite1) sprite_destroy(this->road_sprite1);
        if (this->road_sprite2) sprite_destroy(this->road_sprite2);
        free(this);
        return NULL;
    }

    this->road_y1 = 0;
    this->road_y2 = -this->road_sprite1->height;

    // Initialize Road
    if (road_load(&this->road_data, road_data_file, 1200, road_bg_color, road_surface_file, track_offset_x, track_offset_y, NULL, loading_ui) != 0) {
        printf("Failed to load road data\n");
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    // Initialize Minimap
    if (minimap_init(&this->minimap, road_map_xpm, &this->road_data, track_offset_x, track_offset_y) != 0) {
        printf("Failed to initialize minimap\n");
        road_destroy(&this->road_data);
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    // Initialize Game Items
    if (items_init(&this->game_items, &this->road_data) != 0) {
        printf("Failed to initialize game items\n");
        minimap_destroy(&this->minimap);
        road_destroy(&this->road_data);
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    // Initialize Player
    int creating_car_index = 0;
    Point player_start_pos = this->road_data.center_points[0];
    creating_car_index++;
    float player_initial_angle_rad = 0.0f;
    if (this->road_data.num_center_points > 1) {
        player_initial_angle_rad = atan2(this->road_data.center_points[1].y - this->road_data.center_points[0].y,
                                         this->road_data.center_points[1].x - this->road_data.center_points[0].x);
    }

    if (player_create(&this->player, player_start_pos, player_initial_angle_rad, &this->road_data, car_choices[car_choice]) != 0) {
        printf("Failed to initialize player\n");
        road_destroy(&this->road_data);
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    // Initialize AI Cars
    this->num_active_ai_cars = 0;
    int ai_xpm_idx = 0;
    for (int i = 0; i < MAX_AI_CARS; i++) {

        while (ai_xpm_idx == car_choice) ai_xpm_idx++;
        ai_xpm_idx %= 6;

        Point ai_start_pos = this->road_data.center_points[0];
        creating_car_index++;

        Vector ai_initial_dir_vec;
        ai_initial_dir_vec.x = cos(player_initial_angle_rad);
        ai_initial_dir_vec.y = sin(player_initial_angle_rad);

        AIDifficulty ai_difficulty = AI_DIFFICULTY_MEDIUM;
        if (difficulty == 0) {
            ai_difficulty = AI_DIFFICULTY_EASY;
        } else if (difficulty == 1) {
            ai_difficulty = AI_DIFFICULTY_MEDIUM;
        } else if (difficulty == 2) {
            ai_difficulty = AI_DIFFICULTY_HARD;
        }

        this->ai_cars[i] = ai_car_create(i, ai_start_pos, ai_initial_dir_vec, ai_difficulty, car_choices[ai_xpm_idx], &this->road_data);
        if (this->ai_cars[i]) {
            this->num_active_ai_cars++;
        } else {
            printf("game_state_create_playing: Failed to create AI car %d.\n", i);
            for (int j = 0; j < i; j++) {
                ai_car_destroy(this->ai_cars[j]);
            }
        }
        ai_xpm_idx++;
    }
    if (this->num_active_ai_cars == 0 && MAX_AI_CARS > 0) {
        printf("game_state_create_playing: No AI cars created.\n");
        player_destroy(&this->player);
        road_destroy(&this->road_data);
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    // Initialize Game State
    this->current_running_state = GAME_SUBSTATE_COUNTDOWN;
    this->timer_count_down = 3.99f;
    this->player_skid_input_active = false;
    this->player_turn_input_sign = 0;
    this->total_laps = MAX_LAPS;
    this->current_lap = 0;
    this->pause_requested = false;

    this->timer_count_down = 3.99f;


    countdownTextComponent = create_text_component("3", gameFont, 0xFFFFFF); 
    if (countdownTextComponent && countdownTextComponent->data) {
        TextElementData *data = (TextElementData *)countdownTextComponent->data;
        countdownTextComponent->x = vbe_mode_info.XResolution / 2 - data->width / 2;
        countdownTextComponent->y = vbe_mode_info.YResolution / 2 - data->height / 2;
    }
    
    return this;
}

void playing_destroy(Game *this) {
    this->base.destroy(&this->base);
}

void playing_draw(Game *this) {
    this->base.draw(&this->base);
}

void playing_process_event(Game *this, EventType event) {
    this->base.process_event(&this->base, event);
}

void playing_update_state(Game *this) {
    this->base.update_state(&this->base);
}

GameRunningState playing_get_current_substate(Game *this) {
    return this->current_running_state;
}

void playing_reset_state(Game *this) {
    this->current_running_state = GAME_SUBSTATE_LOADING;
}

