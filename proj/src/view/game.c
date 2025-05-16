#include <lcom/lcf.h>
#include "game.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern int timer_counter;

static void transform_world_to_screen(const Player *player_camera_view, Point world_p, Point *screen_p) {
    if (!player_camera_view || !screen_p) return;

    // Car's center is the reference for the view's "center" on screen
    Point car_center_world = player_camera_view->world_position_car_center;
    float view_angle_rad = atan2(player_camera_view->forward_direction.y, player_camera_view->forward_direction.x);

    // 1. Translate world point to be relative to the car's center
    float translated_x = world_p.x - car_center_world.x;
    float translated_y = world_p.y - car_center_world.y;

    // 2. Rotate this translated point opposite to the car's view angle
    // If car view is rotated by 'view_angle_rad', world rotates by '-view_angle_rad'
    float cos_inv_angle = cos(-view_angle_rad);
    float sin_inv_angle = sin(-view_angle_rad);

    float view_space_x = translated_x * cos_inv_angle - translated_y * sin_inv_angle;
    float view_space_y = translated_x * sin_inv_angle + translated_y * cos_inv_angle;

    // 3. Translate view space coordinates to screen coordinates
    // The car (view center) is drawn at the screen's actual center.
    screen_p->x = (player_camera_view->view_width / 2.0f) + view_space_x;
    // Screen Y is often inverted relative to world Y (if world Y goes up, screen Y goes down)
    screen_p->y = (player_camera_view->view_height / 2.0f) - view_space_y;
}

static void draw_player_car(Player *player) {
    if (player->sprite) {
        int screen_x = player->view_width / 2;
        int screen_y = player->view_height / 2;
        screen_x -= player->sprite->width / 2;
        screen_y -= player->sprite->height / 2;
        sprite_draw_xpm(player->sprite, screen_x, screen_y, true);
        printf("Drawing Player Car at screen center (Rotation/Visuals TODO)\n");
    }
}

static void draw_ai_car(AICar *ai, const Player *player_camera_view) {
    if (ai->sprite) {
        Point screen_pos;

        transform_world_to_screen(player_camera_view, ai->world_position, &screen_pos);
        if (screen_pos.x + ai->sprite->width < 0 || screen_pos.x > player_camera_view->view_width ||
            screen_pos.y + ai->sprite->height < 0 || screen_pos.y > player_camera_view->view_height) {
            return;

        }
        sprite_draw_xpm(ai->sprite, (int)screen_pos.x - ai->sprite->width/2, (int)screen_pos.y - ai->sprite->height/2, true);
        printf("Drawing AI Car %d (Visuals TODO)\n", ai->id);
    }
}

static void playing_process_event_internal(GameState *base, EventType event) {
    Game *this = (Game *)base;
    if (!this) return;

    /*
    if (this->current_running_state == GAME_SUBSTATE_PAUSED || this->current_running_state == GAME_SUBSTATE_FINISHED_RACE) {
        return;
    }
     */

    if (event == EVENT_TIMER) {
        this->base.update_state(base);
    } else if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case LEFT_ARROW:
              this->player_turn_input_sign = -1;
            break;
            case LEFT_ARROW_BREAK:
              if (this->player_turn_input_sign == -1) {
                this->player_turn_input_sign = 0;
              }
            break;
            case RIGHT_ARROW:
              this->player_turn_input_sign = 1;
            break;
            case RIGHT_ARROW_BREAK:
              if (this->player_turn_input_sign == 1) {
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
        if (this->current_running_state == GAME_SUBSTATE_PAUSED || this->current_running_state == GAME_SUBSTATE_FINISHED_RACE) {
          /*
            if (base->handle_mouse_input(base, (void (*)(GameState *))select_difficulty_clean_dirty_mouse_internal, select_difficulty_is_mouse_over, &this->selectedOption)) {
                this->chosenLevel = DIFFICULTY_SELECTED;
            }
           */
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
            if (timer_counter % 120 == 0) {
                printf("Countdown Timer: %d\n", this->timer_count_down);
                this->timer_count_down--;
            }
            if (this->timer_count_down == 0) {
              this->current_running_state = GAME_SUBSTATE_PLAYING;
            }
            break;
        case GAME_SUBSTATE_PLAYING:
            player_handle_turn_input(&this->player, this->player_turn_input_sign);
            player_update(&this->player, &this->road_data, this->player_skid_input_active, delta_time);

            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i]) {
                    ai_car_update(this->ai_cars[i], &this->road_data, &this->player, NULL, 0, delta_time);
                    if (timer_counter % 60 == 0) {
                        printf("AI Car %d Position: (%d, %d)\n", this->ai_cars[i]->id,
                               (int)this->ai_cars[i]->world_position.x, (int)this->ai_cars[i]->world_position.y);
                    }
                }
            }
            // TODO: Collision detection, lap counting, finish conditions
            break;
        case GAME_SUBSTATE_PAUSED:
            // No game logic updates
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

    road_destroy(&this->road_data);
    player_destroy(&this->player);

    for (int i = 0; i < MAX_AI_CARS; i++) {
        ai_car_destroy(this->ai_cars[i]);
    }

    free(this);
}

static void playing_draw_internal(GameState *base) {
    Game *this = (Game *)base;

    if (this->current_running_state == GAME_SUBSTATE_LOADING) { /* Draw loading screen */ return; }

    if (this->current_running_state == GAME_SUBSTATE_PLAYING ||
        this->current_running_state == GAME_SUBSTATE_FINISHED_RACE ||
        this->current_running_state == GAME_SUBSTATE_COUNTDOWN) {

        // road_draw(&this->road_data);

        for (int i = 0; i < this->num_active_ai_cars; ++i) {
            if (this->ai_cars[i]) {
                draw_ai_car(this->ai_cars[i], &this->player);
            }
        }
        draw_player_car(&this->player);
    }

    if (this->current_running_state == GAME_SUBSTATE_PAUSED) { /* Draw Pause Menu Overlay */ }
    else if (this->current_running_state == GAME_SUBSTATE_FINISHED_RACE) { /* Draw Race Finished UI */ }
}

Game *game_state_create_playing(int difficulty, xpm_map_t *player_xpm, char *road_data_file, xpm_map_t *road_xpm, xpm_map_t *finish_xpm) {
    Game *this = (Game *) malloc(sizeof(Game));
    if (this == NULL) {
        return NULL;
    }
    memset(this, 0, sizeof(Game));

    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))playing_draw_internal;
    this->base.process_event = playing_process_event_internal;
    this->base.update_state = playing_update_internal;
    this->base.destroy = playing_destroy_internal;

    this->current_running_state = GAME_SUBSTATE_LOADING;

    // Initialize Road
    if (road_load(&this->road_data, road_data_file, 200, 0x228B22, (xpm_map_t) road_xpm, (xpm_map_t) finish_xpm) != 0) {
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

    if (player_create(&this->player, player_start_pos, player_initial_angle_rad, &this->road_data, player_xpm) != 0) {
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
    this->player_skid_input_active = 0;
    this->player_turn_input_sign = 0;
    this->current_lap = 0;
    this->pause_requested = false;

    this->timer_count_down = 3;

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

