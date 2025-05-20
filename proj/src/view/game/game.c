#include <lcom/lcf.h>

#include "game.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern int timer_counter;
static UIComponent *countdownTextComponent = NULL;
extern Font *gameFont;

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
        renderer_draw_player_car(&this->player);

        // TODO: Draw HUD (laps, speed, timer)
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
            break;
            case SPACEBAR_BREAK:
              this->player_skid_input_active = false;
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
                    fprintf(stderr, "Error loading countdown text: %s\n", text_data->text);
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

    switch(this->current_running_state) {
        case GAME_SUBSTATE_LOADING:
            // if (assets_loaded()) this->current_running_state = GAME_RUN_STATE_COUNTDOWN;
            break;
        case GAME_SUBSTATE_COUNTDOWN:
            update_countdown(this, delta_time);
            break;
        case GAME_SUBSTATE_PLAYING:
            this->road_y1 += 2;
            this->road_y2 += 2;

            if (this->road_y1 >= (int)vbe_mode_info.YResolution)
                this->road_y1 = -this->road_sprite1->height;

            if (this->road_y2 >= (int)vbe_mode_info.YResolution)
                this->road_y2 = -this->road_sprite2->height;

            player_handle_turn_input(&this->player, this->player_turn_input_sign);
            player_update(&this->player, &this->road_data, this->player_skid_input_active, delta_time);
            if (timer_counter % 60 == 0) {
                printf("Player Position: (%d, %d)\n", (int)this->player.world_position_car_center.x, (int)this->player.world_position_car_center.y);
            }

            this->current_lap = this->player.current_lap;

            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i]) {
                    ai_car_update(this->ai_cars[i], &this->road_data, &this->player, NULL, 0, delta_time);
                    if (timer_counter % 60 == 0) {
                        // printf("AI Car %d Position: (%d, %d)\n", this->ai_cars[i]->id, (int)this->ai_cars[i]->world_position.x, (int)this->ai_cars[i]->world_position.y);
                    }
                }
            }
            // TODO: Collision detection, lap counting, finish conditions

            if (this->current_lap > this->player.total_laps) {
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

    road_destroy(&this->road_data);
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

Game *game_state_create_playing(int difficulty, int car_choice, char *road_data_file, char *road_surface_file) {
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

    xpm_map_t car_xpms[4] = { (xpm_map_t)pink_car_xpm, (xpm_map_t)red_car_xpm,
                              (xpm_map_t)orange_car_xpm, (xpm_map_t)blue_car_xpm };
    this->playerCar.car_sprite = sprite_create_xpm(car_xpms[car_choice], 0, 0, 0, 0);
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
    if (road_load(&this->road_data, road_data_file, 700, 0x8EC940, road_surface_file, loading_ui) != 0) {
        printf("Failed to load road data\n");
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    // Initialize Player
    Point player_start_pos = this->road_data.start_point;
    float player_initial_angle_rad = 0.0f;
    if (this->road_data.num_center_points > 1) {
        player_initial_angle_rad = atan2(this->road_data.center_points[1].y - this->road_data.center_points[0].y,
                                         this->road_data.center_points[1].x - this->road_data.center_points[0].x);
    }

    if (player_create(&this->player, player_start_pos, player_initial_angle_rad, &this->road_data, car_xpms[car_choice]) != 0) {
        printf("Failed to initialize player\n");
        road_destroy(&this->road_data);
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    // Initialize AI Cars
    this->num_active_ai_cars = 0;
    for (int i = 0; i < MAX_AI_CARS; i++) {
        Point ai_start_pos = this->road_data.start_point;
        ai_start_pos.x -= (i + 1) * 30.0f;
        ai_start_pos.y += (i % 2 == 0 ? -1 : 1) * 20.0f;

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

        this->ai_cars[i] = ai_car_create(i, ai_start_pos, ai_initial_dir_vec, ai_difficulty, NULL, &this->road_data);
        if (this->ai_cars[i]) {
            this->num_active_ai_cars++;
        } else {
            fprintf(stderr, "game_state_create_playing: Failed to create AI car %d.\n", i);
            for (int j = 0; j < i; j++) {
                ai_car_destroy(this->ai_cars[j]);
            }
        }
    }
    if (this->num_active_ai_cars == 0 && MAX_AI_CARS > 0) {
        fprintf(stderr, "game_state_create_playing: No AI cars created.\n");
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

