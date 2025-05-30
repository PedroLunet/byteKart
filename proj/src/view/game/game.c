#include <lcom/lcf.h>
#include <string.h>

#include "game.h"
#include "../utils/text_renderer.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern int timer_counter;
static UIComponent *countdownText = NULL;
extern Font *gameFont;
extern const xpm_map_t car_choices[6];

static char scancode_to_char(uint8_t scancode);
static void create_name_input_ui(Game *this);
static void update_name_input_display(Game *this);
static void calculate_final_race_positions(Game *this, RaceResult *results, int *total_results);

static char scancode_to_char(uint8_t scancode) {
    switch (scancode) {
        case 0x1E: return 'A';  
        case 0x30: return 'B';  
        case 0x2E: return 'C';  
        case 0x20: return 'D';  
        case 0x12: return 'E';  
        case 0x21: return 'F';  
        case 0x22: return 'G';  
        case 0x23: return 'H';  
        case 0x17: return 'I';  
        case 0x24: return 'J';  
        case 0x25: return 'K';  
        case 0x26: return 'L';  
        case 0x32: return 'M';  
        case 0x31: return 'N';  
        case 0x18: return 'O';  
        case 0x19: return 'P';  
        case 0x10: return 'Q';  
        case 0x13: return 'R';  
        case 0x1F: return 'S';  
        case 0x14: return 'T';  
        case 0x16: return 'U';  
        case 0x2F: return 'V';  
        case 0x11: return 'W';  
        case 0x2D: return 'X';  
        case 0x15: return 'Y';  
        case 0x2C: return 'Z';  
        
        case 0x02: return '1';  
        case 0x03: return '2';  
        case 0x04: return '3';  
        case 0x05: return '4';  
        case 0x06: return '5';  
        case 0x07: return '6';  
        case 0x08: return '7';  
        case 0x09: return '8';  
        case 0x0A: return '9';  
        case 0x0B: return '0';  
        
        default: return '\0';
    }
}

static UIComponent *display_player_lap(Game *this) {
    if (!this) return NULL;
    
    char lap_string[16];
    sprintf(lap_string, "Lap: %d/%d", this->player.current_lap, this->player.total_laps);
    
    UIComponent *lapText = create_text_component(lap_string, gameFont, 0xFFFFFF);
    if (lapText && lapText->data) {
        lapText->x = 20;
        lapText->y = 60;
    }
    
    return lapText;
}

static UIComponent *display_wrong_direction_warning(Game *this) {
    if (!this || !this->is_going_wrong_direction) return NULL;
    
    char warning_text[] = "WRONG WAY!";
    uint32_t color = 0xFF0000; 
    
    UIComponent *warningText = create_text_component(warning_text, gameFont, color);
    if (warningText && warningText->data) {
        TextElementData *data = (TextElementData *)warningText->data;

        int scale = 4;
        int orig_width = data->width;
        int orig_height = data->height;
        int scaled_width = orig_width * scale;
        int scaled_height = orig_height * scale;
        
        uint32_t *scaled_buffer = malloc(scaled_width * scaled_height * sizeof(uint32_t));
        if (scaled_buffer) {
            for (int y = 0; y < orig_height; y++) {
                for (int x = 0; x < orig_width; x++) {
                    uint32_t pixel = data->pixel_data[y * orig_width + x];
                    
                    for (int sy = 0; sy < scale; sy++) {
                        for (int sx = 0; sx < scale; sx++) {
                            int scaled_x = x * scale + sx;
                            int scaled_y = y * scale + sy;
                            if (scaled_x < scaled_width && scaled_y < scaled_height) {
                                scaled_buffer[scaled_y * scaled_width + scaled_x] = pixel;
                            }
                        }
                    }
                }
            }
            
            free(data->pixel_data);
            data->pixel_data = scaled_buffer;
            data->width = scaled_width;
            data->height = scaled_height;
        }

        warningText->x = vbe_mode_info.XResolution / 2 - data->width / 2;
        warningText->y = vbe_mode_info.YResolution / 2 - 100; 
    }
    
    return warningText;
}

static UIComponent *display_player_position(Game *this) {
    if (!this || this->current_total_racers == 0) return NULL;
    
    int player_position = 0;
    for (int i = 0; i < this->current_total_racers; i++) {
        if (strcmp(this->current_race_positions[i].name, "Player") == 0) {
            player_position = this->current_race_positions[i].position;
            break;
        }
    }
    
    if (player_position == 0) return NULL; 
    
    char position_text[8];
    switch (player_position) {
        case 1: sprintf(position_text, "1st"); break;
        case 2: sprintf(position_text, "2nd"); break;
        case 3: sprintf(position_text, "3rd"); break;
        default: sprintf(position_text, "%dth", player_position); break;
    }
    
    uint32_t color = (player_position == 1) ? 0xFFD700 : 
                     (player_position == 2) ? 0xC0C0C0 : 
                     (player_position == 3) ? 0xCD7F32 : 
                     0xFFFFFF;                            
    
    int orig_width = 0;
    int orig_height = 0;
    for (int i = 0; position_text[i] != '\0'; i++) {
        GlyphData glyphData;
        if (font_get_glyph_data(gameFont, position_text[i], &glyphData)) {
            orig_width += glyphData.xadvance;
            if (glyphData.height > orig_height) {
                orig_height = glyphData.height;
            }
        }
    }
    
    if (orig_width == 0 || orig_height == 0) return NULL;
    
    int scale = 3;
    int scaled_width = orig_width * scale;
    int scaled_height = orig_height * scale;
    
    uint32_t *orig_buffer = malloc(orig_width * orig_height * sizeof(uint32_t));
    if (!orig_buffer) return NULL;
    
    for (int i = 0; i < orig_width * orig_height; i++) {
        orig_buffer[i] = 0x00000000;
    }
    
    if (load_text(position_text, 0, 0, color, gameFont, orig_buffer, orig_width) != 0) {
        free(orig_buffer);
        return NULL;
    }
    
    uint32_t *scaled_buffer = malloc(scaled_width * scaled_height * sizeof(uint32_t));
    if (!scaled_buffer) {
        free(orig_buffer);
        return NULL;
    }
    
    for (int y = 0; y < orig_height; y++) {
        for (int x = 0; x < orig_width; x++) {
            uint32_t pixel = orig_buffer[y * orig_width + x];
            
            for (int sy = 0; sy < scale; sy++) {
                for (int sx = 0; sx < scale; sx++) {
                    int scaled_x = x * scale + sx;
                    int scaled_y = y * scale + sy;
                    if (scaled_x < scaled_width && scaled_y < scaled_height) {
                        scaled_buffer[scaled_y * scaled_width + scaled_x] = pixel;
                    }
                }
            }
        }
    }
    
    free(orig_buffer);
    
    UIComponent *positionText = create_text_component(position_text, gameFont, color);
    if (!positionText) {
        free(scaled_buffer);
        return NULL;
    }

    TextElementData *data = (TextElementData *)positionText->data;
    if (data->pixel_data) {
        free(data->pixel_data);
    }
    data->width = scaled_width;
    data->height = scaled_height;
    data->pixel_data = scaled_buffer;

    positionText->x = vbe_mode_info.XResolution - scaled_width - 50;
    positionText->y = 10;
    
    return positionText;
}

static void playing_draw_internal(GameState *base) {
    Game *this = (Game *)base;

    // Clear the screen
    renderer_clear_buffer(this->road_data.background_color);

    if (this->current_running_state == GAME_SUBSTATE_PAUSED) {
        if (this->pauseMenu) {
            vg_draw_rectangle(0, 0, vbe_mode_info.XResolution, vbe_mode_info.YResolution, 0x80000000);
            pause_draw(this->pauseMenu);
        }
        return;
    }

    if (this->current_running_state == GAME_SUBSTATE_NAME_INPUT) {
        if (this->nameInputContainer) {
            draw_ui_component(this->nameInputContainer);
        }
        return;
    }

    if (this->current_running_state == GAME_SUBSTATE_PLAYING ||
        this->current_running_state == GAME_SUBSTATE_COUNTDOWN ||
        this->current_running_state == GAME_SUBSTATE_PLAYER_FINISHED ||
        this->current_running_state == GAME_SUBSTATE_RACE_FINISH_DELAY ||
        this->current_running_state == GAME_SUBSTATE_FINISHED_RACE) {

        renderer_draw_road(&this->road_data, &this->player);

        for (int i = 0; i < this->num_active_ai_cars; ++i) {
            if (this->ai_cars[i]) {
                renderer_draw_ai_car(this->ai_cars[i], &this->player);
            }
        }
        renderer_draw_player_car(&this->player, this->player_skid_input_active, this->player_skid_input_sign, this->precomputed_cos_skid, this->precomputed_sin_skid);

        UIComponent *timerText = NULL;
        if (this->current_running_state == GAME_SUBSTATE_COUNTDOWN) {
            timerText = display_cronometer(0.0); // Timer to 0 when countdown is running

            if (timerText) {
                draw_ui_component(timerText);
                destroy_ui_component(timerText);
            }


        }

        if (timerText) {
                draw_ui_component(timerText);
                destroy_ui_component(timerText);
        }

        if (countdownText) {
            draw_ui_component(countdownText);
        
        UIComponent *positionText = display_player_position(this);
        if (positionText) {
            draw_ui_component(positionText);
            destroy_ui_component(positionText);
        }

        UIComponent *lapText = display_player_lap(this);
        if (lapText) {
            draw_ui_component(lapText);
            destroy_ui_component(lapText);
        }
        
        UIComponent *wrongWayText = display_wrong_direction_warning(this);
        if (wrongWayText) {
            draw_ui_component(wrongWayText);
            destroy_ui_component(wrongWayText);
        }
    }

        // TODO: Draw HUD (laps, speed)

        if (this->current_running_state == GAME_SUBSTATE_PLAYER_FINISHED) {
            vg_draw_rectangle(0, 0, vbe_mode_info.XResolution, 120, 0x80000000);
            
            if (gameFont) {
                char finish_message[100];
                sprintf(finish_message, "Race Finished! Time: %.2f seconds", this->player_finish_time);
                
                int text_width = 0;
                int text_height = 0;
                for (int i = 0; finish_message[i] != '\0'; i++) {
                    GlyphData glyphData;
                    if (font_get_glyph_data(gameFont, finish_message[i], &glyphData)) {
                        text_width += glyphData.xadvance;
                        if (glyphData.height > text_height) {
                            text_height = glyphData.height;
                        }
                    }
                }
                
                if (text_width > 0 && text_height > 0) {
                    uint32_t *text_buffer = malloc(text_width * text_height * sizeof(uint32_t));
                    if (text_buffer) {

                        for (int i = 0; i < text_width * text_height; i++) {
                            text_buffer[i] = 0x00000000;
                        }
                        
                        if (load_text(finish_message, 0, 0, 0xFFFFFF, gameFont, text_buffer, text_width) == 0) {
                            int text_x = (vbe_mode_info.XResolution - text_width) / 2;
                            vg_draw_text(text_buffer, text_width, text_x, 20, text_height, text_width);
                        }
                        free(text_buffer);
                    }
                }
                
                char wait_message[] = "Waiting for other cars to finish...";
                int wait_text_width = 0;
                int wait_text_height = 0;
                for (int i = 0; wait_message[i] != '\0'; i++) {
                    GlyphData glyphData;
                    if (font_get_glyph_data(gameFont, wait_message[i], &glyphData)) {
                        wait_text_width += glyphData.xadvance;
                        if (glyphData.height > wait_text_height) {
                            wait_text_height = glyphData.height;
                        }
                    }
                }
                
                if (wait_text_width > 0 && wait_text_height > 0) {
                    uint32_t *wait_buffer = malloc(wait_text_width * wait_text_height * sizeof(uint32_t));
                    if (wait_buffer) {
                        for (int i = 0; i < wait_text_width * wait_text_height; i++) {
                            wait_buffer[i] = 0x00000000;
                        }
                        
                        if (load_text(wait_message, 0, 0, 0xFFFF00, gameFont, wait_buffer, wait_text_width) == 0) {
                            int wait_text_x = (vbe_mode_info.XResolution - wait_text_width) / 2;
                            vg_draw_text(wait_buffer, wait_text_width, wait_text_x, 60, wait_text_height, wait_text_width);
                        }
                        free(wait_buffer);
                    }
                }
            }
        }
        if (this->current_running_state == GAME_SUBSTATE_PLAYING) {
            UIComponent *timerText = display_cronometer(this->cronometer_time);
            if (timerText) {
                draw_ui_component(timerText);
                destroy_ui_component(timerText);
            }
            
            UIComponent *positionText = display_player_position(this);
            if (positionText) {
                draw_ui_component(positionText);
                destroy_ui_component(positionText);
            }

            UIComponent *lapText = display_player_lap(this);
            if (lapText) {
                draw_ui_component(lapText);
                destroy_ui_component(lapText);
            }
            
            UIComponent *wrongWayText = display_wrong_direction_warning(this);
            if (wrongWayText) {
                draw_ui_component(wrongWayText);
                destroy_ui_component(wrongWayText);
            }
        }
    }

    if (this->current_running_state == GAME_SUBSTATE_LOADING) { 
        return; 
    }
    
    else if (this->current_running_state == GAME_SUBSTATE_FINISHED_RACE) { 
        if (this->finishRaceMenu) {
            finish_race_draw(this->finishRaceMenu);
        }
    }
}

static void playing_process_event_internal(GameState *base, EventType event) {
    Game *this = (Game *)base;
    if (!this) return;

    if (this->current_running_state == GAME_SUBSTATE_PAUSED && this->pauseMenu) {
        pause_process_event(this->pauseMenu, event);
        PauseSubstate pauseState = pause_get_current_substate(this->pauseMenu);
        if (pauseState == PAUSE_RESUME) {
            this->current_running_state = GAME_SUBSTATE_PLAYING;
            this->pause_requested = false;
            pause_menu_destroy(this->pauseMenu);
            this->pauseMenu = NULL;
            this->base.draw(base); 
        } 
        else if (pauseState == PAUSE_MAIN_MENU) {
            printf("Returning to main menu from pause menu\n");
            this->current_running_state = GAME_SUBSTATE_BACK_TO_MENU;
            pause_menu_destroy(this->pauseMenu);
            this->pauseMenu = NULL;
        }
        return;
    }

    if (this->current_running_state == GAME_SUBSTATE_FINISHED_RACE && this->finishRaceMenu) {
        finish_race_process_event(this->finishRaceMenu, event);
        FinishRaceSubstate finishRaceState = finish_race_get_current_substate(this->finishRaceMenu);
        if (finishRaceState == FINISH_RACE_MAIN_MENU) {
            if (this->finishRaceMenu->selectedOption == 0) {
                printf("Restarting the game with same settings\n");
                this->replay_requested = true;
            } else {
                printf("Returning to main menu from finish race menu\n");
                this->replay_requested = false;
            }
            this->current_running_state = GAME_SUBSTATE_BACK_TO_MENU;
            finish_race_menu_destroy(this->finishRaceMenu);
            this->finishRaceMenu = NULL;
        }
        return;
    }

    if (event == EVENT_TIMER) {
        this->base.update_state(base);
        this->base.draw(base);
    } else if (event == EVENT_KEYBOARD) {
        if (this->current_running_state == GAME_SUBSTATE_NAME_INPUT) {
            switch (scancode) {
                case ENTER_KEY:
                    if (this->name_length > 0) {
                        this->player_name[this->name_length] = '\0';

                        add_entry_to_leaderboard(leaderboard_entries, &count_leaderboard_entries, 
                                                this->player_name, this->player_finish_time);

                        destroy_ui_component(this->nameInputContainer);
                        this->nameInputContainer = NULL;
                        this->nameInputText = NULL;

                        RaceResult final_results[MAX_AI_CARS + 1];
                        int total_results = 0;
                        calculate_final_race_positions(this, final_results, &total_results);
                        
                        this->finishRaceMenu = finish_race_menu_create(final_results, total_results);
                        this->current_running_state = GAME_SUBSTATE_FINISHED_RACE;
                        this->base.draw(base);
                    }
                    break;
                    
                case BACKSPACE_KEY:
                    if (this->name_length > 0) {
                        this->name_length--;
                        this->player_name[this->name_length] = '\0';
                        update_name_input_display(this);
                        this->base.draw(base);
                    }
                    break;
                    
                case ESC_BREAKCODE:
                    destroy_ui_component(this->nameInputContainer);
                    this->nameInputContainer = NULL;
                    this->nameInputText = NULL;

                    RaceResult final_results[MAX_AI_CARS + 1];
                    int total_results = 0;
                    calculate_final_race_positions(this, final_results, &total_results);
                    
                    this->finishRaceMenu = finish_race_menu_create(final_results, total_results);
                    this->current_running_state = GAME_SUBSTATE_FINISHED_RACE;
                    this->base.draw(base);
                    break;
                    
                default:
                    if (this->name_length < MAX_NAME_LENGTH - 1) {
                        char input_char = scancode_to_char(scancode);
                        if (input_char != '\0') {
                            this->player_name[this->name_length] = input_char;
                            this->name_length++;
                            update_name_input_display(this);
                            this->base.draw(base);
                        }
                    }
                    break;
            }
            return;
        }
        
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
                  if (!this->pauseMenu) {
                      this->pauseMenu = pause_menu_create();
                  }
                  this->current_running_state = GAME_SUBSTATE_PAUSED;
                  this->pause_requested = true;
                  this->base.draw(base);
              }
              else if (this->current_running_state == GAME_SUBSTATE_PAUSED) {
                  this->current_running_state = GAME_SUBSTATE_PLAYING;
                  this->pause_requested = false;
                  if (this->pauseMenu) {
                      pause_menu_destroy(this->pauseMenu);
                      this->pauseMenu = NULL;
                  }
                  this->base.draw(base); 
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
        if (countdownText) {
            char countdown_str[5];
            int count = (int)this->timer_count_down;
            if (count > 0 && count <= 3) {
                sprintf(countdown_str, "%d", count);
            } else if (count <= 0) {
                sprintf(countdown_str, "GO!");
            } else {
                countdown_str[0] = '\0';
            }
            TextElementData *text_data = (TextElementData *)countdownText->data;
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
                
                text_data->pixel_data = malloc(text_data->width * text_data->height * sizeof(uint32_t));
                if (text_data->pixel_data == NULL) {
                    fprintf(stderr, "Error allocating memory for countdown text pixel data\n");
                    return;
                }
                for (int i = 0; i < text_data->width * text_data->height; i++) {
                    text_data->pixel_data[i] = 0x00000000; 
                }
                
                if (load_text(text_data->text, 0, 0, text_data->color, text_data->font, text_data->pixel_data, text_data->width) != 0) {
                    fprintf(stderr, "Error loading countdown text: %s\n", text_data->text);
                }
                countdownText->x = vbe_mode_info.XResolution / 2 - text_data->width / 2;
                countdownText->y = vbe_mode_info.YResolution / 2 - text_data->height / 2;
            }
        }
    }
    if (this->timer_count_down <= 0) {
        this->current_running_state = GAME_SUBSTATE_PLAYING;
        if (countdownText) {
            destroy_ui_component(countdownText);
            countdownText = NULL;
        }
    }
}

static int calculate_race_position_score(int current_lap, int current_segment) {
    return (current_lap * 27000) + current_segment; 
}

static void calculate_final_race_positions(Game *this, RaceResult *results, int *total_results) {
    typedef struct {
        int score;
        int position;
        const char* name;
        int id;
        int lap;
        int segment;
        float race_time;
        bool finished;
    } RaceEntry;
    
    RaceEntry entries[MAX_AI_CARS + 1]; 
    int total_entries = 0;
    
    // Add player entry
    entries[total_entries].score = calculate_race_position_score(this->player.current_lap, this->player.current_road_segment_idx);
    entries[total_entries].name = (this->name_length > 0) ? this->player_name : "Player";
    entries[total_entries].id = 0;
    entries[total_entries].lap = this->player.current_lap;
    entries[total_entries].segment = this->player.current_road_segment_idx;
    entries[total_entries].race_time = this->player_finish_time;
    entries[total_entries].finished = this->player_has_finished;
    total_entries++;

    // Add AI car entries
    for (int i = 0; i < this->num_active_ai_cars; ++i) {
        if (this->ai_cars[i]) {
            entries[total_entries].score = calculate_race_position_score(this->ai_cars[i]->current_lap, this->ai_cars[i]->current_road_segment_idx);
            entries[total_entries].name = "AI Car";
            entries[total_entries].id = this->ai_cars[i]->id;
            entries[total_entries].lap = this->ai_cars[i]->current_lap;
            entries[total_entries].segment = this->ai_cars[i]->current_road_segment_idx;
            entries[total_entries].race_time = this->ai_cars[i]->finish_time;
            entries[total_entries].finished = this->ai_cars[i]->has_finished;
            total_entries++;
        }
    }

    for (int i = 0; i < total_entries - 1; i++) {
        for (int j = 0; j < total_entries - i - 1; j++) {
            bool should_swap = false;

            if (entries[j].finished && entries[j + 1].finished) {
                should_swap = (entries[j].race_time > entries[j + 1].race_time);
            }

            else if (!entries[j].finished && entries[j + 1].finished) {
                should_swap = true;
            }
            else if (!entries[j].finished && !entries[j + 1].finished) {
                should_swap = (entries[j].score < entries[j + 1].score);
            }
            
            if (should_swap) {
                RaceEntry temp = entries[j];
                entries[j] = entries[j + 1];
                entries[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < total_entries; i++) {
        results[i].position = i + 1;
        if (strcmp(entries[i].name, "Player") == 0 || 
            (this->name_length > 0 && strcmp(entries[i].name, this->player_name) == 0)) {
            strcpy(results[i].name, (this->name_length > 0) ? this->player_name : "Player");
        } else {
            sprintf(results[i].name, "AI Car %d", entries[i].id);
        }
        results[i].id = entries[i].id;
        results[i].lap = entries[i].lap;
        results[i].segment = entries[i].segment;
        results[i].score = entries[i].score;
        results[i].race_time = entries[i].race_time;
    }
    
    *total_results = total_entries;
}

static void calculate_current_race_positions(Game *this, RaceResult *results, int *total_results) {
    typedef struct {
        int score;
        int position;
        const char* name;
        int id;
        int lap;
        int segment;
    } RaceEntry;
    
    RaceEntry entries[MAX_AI_CARS + 1]; 
    int total_entries = 0;
    
    // Add player entry
    entries[total_entries].score = calculate_race_position_score(this->player.current_lap, this->player.current_road_segment_idx);
    entries[total_entries].name = (this->name_length > 0) ? this->player_name : "Player";
    entries[total_entries].id = 0;
    entries[total_entries].lap = this->player.current_lap;
    entries[total_entries].segment = this->player.current_road_segment_idx;
    total_entries++;
    
    // Add AI car entries
    for (int i = 0; i < this->num_active_ai_cars; ++i) {
        if (this->ai_cars[i]) {
            entries[total_entries].score = calculate_race_position_score(this->ai_cars[i]->current_lap, this->ai_cars[i]->current_road_segment_idx);
            entries[total_entries].name = "AI Car";
            entries[total_entries].id = this->ai_cars[i]->id;
            entries[total_entries].lap = this->ai_cars[i]->current_lap;
            entries[total_entries].segment = this->ai_cars[i]->current_road_segment_idx;
            total_entries++;
        }
    }
    
    for (int i = 0; i < total_entries - 1; i++) {
        for (int j = 0; j < total_entries - i - 1; j++) {
            if (entries[j].score < entries[j + 1].score) {
                RaceEntry temp = entries[j];
                entries[j] = entries[j + 1];
                entries[j + 1] = temp;
            }
        }
    }

    for (int i = 0; i < total_entries; i++) {
        results[i].position = i + 1;
        if (strcmp(entries[i].name, "Player") == 0 || 
            (this->name_length > 0 && strcmp(entries[i].name, this->player_name) == 0)) {
            strcpy(results[i].name, (this->name_length > 0) ? this->player_name : "Player");
        } else {
            sprintf(results[i].name, "AI Car %d", entries[i].id);
        }
        results[i].id = entries[i].id;
        results[i].lap = entries[i].lap;
        results[i].segment = entries[i].segment;
        results[i].score = entries[i].score;
        results[i].race_time = 0.0f; 
    }
    
    *total_results = total_entries;
}

static void print_race_positions(Game *this) {
    RaceResult current_positions[MAX_AI_CARS + 1];
    int total_results = 0;
    
    calculate_current_race_positions(this, current_positions, &total_results);

    printf("\n=== RACE POSITIONS ===\n");
    for (int i = 0; i < total_results; i++) {
        printf("%d. %s - Lap %d, Segment %d (Score: %d)\n", 
               current_positions[i].position, current_positions[i].name, 
               current_positions[i].lap, current_positions[i].segment, current_positions[i].score);
    }
    printf("======================\n\n");
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
            this->race_timer_s += delta_time;
            
            this->road_y1 += 2;
            this->road_y2 += 2;

            if (this->road_y1 >= (int)vbe_mode_info.YResolution)
                this->road_y1 = -this->road_sprite1->height;

            if (this->road_y2 >= (int)vbe_mode_info.YResolution)
                this->road_y2 = -this->road_sprite2->height;
            
            this->cronometer_time += delta_time;

            player_handle_turn_input(&this->player, this->player_turn_input_sign);
            player_update(&this->player, &this->road_data, this->player_skid_input_active, delta_time);
            if (timer_counter % 60 == 0) {
                // printf("Player Position: (%d, %d)\n", (int)this->player.world_position_car_center.x, (int)this->player.world_position_car_center.y);
            }

            this->current_lap = this->player.current_lap;

            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i]) {
                    ai_car_update(this->ai_cars[i], &this->road_data, &this->player, NULL, 0, delta_time);
                    
                    if (this->ai_cars[i]->has_finished && this->ai_cars[i]->finish_time == 0.0f) {
                        ai_car_set_finish_time(this->ai_cars[i], this->race_timer_s);
                        printf("AI Car %d finished at time: %.2f seconds\n", this->ai_cars[i]->id, this->race_timer_s);
                    }
                    
                    if (timer_counter % 60 == 0) {
                        // printf("AI Car %d Position: (%d, %d)\n", this->ai_cars[i]->id, (int)this->ai_cars[i]->world_position.x, (int)this->ai_cars[i]->world_position.y);
                    }
                }
            }
            // TODO: Collision detection, lap counting, finish conditions
            for (int i = 0; i < this->num_active_ai_cars; ++i) {
        		if (this->ai_cars[i]) {
            		if (obb_check_collision_obb_vs_obb(&this->player.obb, &this->ai_cars[i]->obb)) {
                		printf("Collision: Player vs AI Car %d\n", this->ai_cars[i]->id);
                		// TODO: Handle Player vs. AI collision response
                		// E.g., player_handle_hard_collision(&game->player, game->player.current_speed * 0.5f);
               			// E.g., ai_car_handle_hard_collision(game->ai_cars[i], game->ai_cars[i]->current_speed * 0.5f);
            		}
        		}
    		}

    		// AI vs. AI Cars
    		for (int i = 0; i < this->num_active_ai_cars; ++i) {
        		if (!this->ai_cars[i]) continue;
        		for (int j = i + 1; j < this->num_active_ai_cars; ++j) {
           			if (!this->ai_cars[j]) continue;
            		if (obb_check_collision_obb_vs_obb(&this->ai_cars[i]->obb, &this->ai_cars[j]->obb)) {
                		// printf("Collision: AI Car %d vs AI Car %d\n", this->ai_cars[i]->id, this->ai_cars[j]->id);
                		// TODO: Handle AI vs. AI collision response
            		}
        		}
    		}

        	int player_seg_idx = this->player.current_road_segment_idx;
   			int search_radius_edges = 10;
    		int N_road_points = this->road_data.num_center_points;

        	for (int i = 0; i < (2 * search_radius_edges + 1); ++i) {
            	int offset = i - search_radius_edges;
            	int seg_to_check = (player_seg_idx + offset % N_road_points + N_road_points) % N_road_points;
            	int next_seg_to_check = (seg_to_check + 1) % N_road_points;

            	Point p0_left = this->road_data.left_edge_points[seg_to_check];
            	Point p1_left = this->road_data.left_edge_points[next_seg_to_check];
            	if (obb_check_collision_obb_vs_line_segment(&this->player.obb, p0_left, p1_left)) {
                	// printf("Collision: Player vs Left Track Edge (segment %d)\n", seg_to_check);
                	// TODO: Handle Player vs. Left Edge collision response
            	}

            	Point p0_right = this->road_data.right_edge_points[seg_to_check];
            	Point p1_right = this->road_data.right_edge_points[next_seg_to_check];
            	if (obb_check_collision_obb_vs_line_segment(&this->player.obb, p0_right, p1_right)) {
                	// printf("Collision: Player vs Right Track Edge (segment %d)\n", seg_to_check);
                	// TODO: Handle Player vs. Right Edge collision response
            	}
        	}

            // Print race positions every 3 seconds (180 frames at 60 FPS)
            if (timer_counter % 180 == 0) {
                print_race_positions(this);
            }

            calculate_current_race_positions(this, this->current_race_positions, &this->current_total_racers);

            int current_player_score = calculate_race_position_score(this->player.current_lap, this->player.current_road_segment_idx);

            if (this->previous_player_score > 0) { 
                if (current_player_score < this->previous_player_score) {
                    this->wrong_direction_timer += delta_time;

                    if (this->wrong_direction_timer >= 1.0f) {
                        this->is_going_wrong_direction = true;
                    }
                } else if (current_player_score > this->previous_player_score) {
                    this->wrong_direction_timer = 0.0f;
                    this->is_going_wrong_direction = false;
                }
            }
            this->previous_player_score = current_player_score;

            if (!this->player_has_finished && this->player.current_lap > this->player.total_laps) {
                this->player_has_finished = true;
                this->player_finish_time = this->race_timer_s;
                this->current_running_state = GAME_SUBSTATE_PLAYER_FINISHED;
                printf("Player finished the race! Final time: %.2f seconds\n", this->player_finish_time);
                printf("Waiting for other cars to finish...\n");
            }

            bool all_cars_finished = true;
            
            // Check player
            if (this->player.current_lap <= this->player.total_laps) {
                all_cars_finished = false;
            }
            
            // Check AI cars
            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i] && this->ai_cars[i]->current_lap <= this->ai_cars[i]->total_laps) {
                    all_cars_finished = false;
                    break;
                }
            }
            
            if (all_cars_finished) {
                this->current_running_state = GAME_SUBSTATE_RACE_FINISH_DELAY;
                this->finish_race_delay_timer = 1.0f; // 1 second delay
                printf("All cars finished the race! Starting delay...\n");
            }

            break;
        case GAME_SUBSTATE_PLAYER_FINISHED:
            this->race_timer_s += delta_time;

            this->road_y1 += 2;
            this->road_y2 += 2;

            if (this->road_y1 >= (int)vbe_mode_info.YResolution)
                this->road_y1 = -this->road_sprite1->height;

            if (this->road_y2 >= (int)vbe_mode_info.YResolution)
                this->road_y2 = -this->road_sprite2->height;
            player_handle_turn_input(&this->player, this->player_turn_input_sign);
            player_update(&this->player, &this->road_data, this->player_skid_input_active, delta_time);

            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i]) {
                    ai_car_update(this->ai_cars[i], &this->road_data, &this->player, NULL, 0, delta_time);
                    
                    if (this->ai_cars[i]->has_finished && this->ai_cars[i]->finish_time == 0.0f) {
                        ai_car_set_finish_time(this->ai_cars[i], this->race_timer_s);
                        printf("AI Car %d finished at time: %.2f seconds\n", this->ai_cars[i]->id, this->race_timer_s);
                    }
                }
            }

            bool all_cars_finished_player_state = true;

            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i] && this->ai_cars[i]->current_lap <= this->ai_cars[i]->total_laps) {
                    all_cars_finished_player_state = false;
                    break;
                }
            }
            
            if (all_cars_finished_player_state) {
                this->current_running_state = GAME_SUBSTATE_RACE_FINISH_DELAY;
                this->finish_race_delay_timer = 1.0f; // 1 second delay
                printf("All remaining cars finished! Starting delay...\n");
            }

            if (timer_counter % 180 == 0) {
                print_race_positions(this);
            }

            calculate_current_race_positions(this, this->current_race_positions, &this->current_total_racers);

            break;
        case GAME_SUBSTATE_RACE_FINISH_DELAY:
            this->road_y1 += 2;
            this->road_y2 += 2;

            if (this->road_y1 >= (int)vbe_mode_info.YResolution)
                this->road_y1 = -this->road_sprite1->height;

            if (this->road_y2 >= (int)vbe_mode_info.YResolution)
                this->road_y2 = -this->road_sprite2->height;

            player_handle_turn_input(&this->player, this->player_turn_input_sign);
            player_update(&this->player, &this->road_data, this->player_skid_input_active, delta_time);

            for (int i = 0; i < this->num_active_ai_cars; ++i) {
                if (this->ai_cars[i]) {
                    ai_car_update(this->ai_cars[i], &this->road_data, &this->player, NULL, 0, delta_time);
                    
                    if (this->ai_cars[i]->has_finished && this->ai_cars[i]->finish_time == 0.0f) {
                        ai_car_set_finish_time(this->ai_cars[i], this->race_timer_s);
                        printf("AI Car %d finished at time: %.2f seconds\n", this->ai_cars[i]->id, this->race_timer_s);
                    }
                }
            }

            this->finish_race_delay_timer -= delta_time;
            if (this->finish_race_delay_timer <= 0.0f) {
                this->current_running_state = GAME_SUBSTATE_NAME_INPUT;
                create_name_input_ui(this);
                printf("Delay finished, showing name input!\n");
            }
            break;
        case GAME_SUBSTATE_NAME_INPUT:
            break;
        case GAME_SUBSTATE_PAUSED:
            break;
        case GAME_SUBSTATE_FINISHED_RACE:
            // Maybe some post-race animation or waiting for input
            break;
        case GAME_SUBSTATE_BACK_TO_MENU:
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

    if (countdownText) {
        destroy_ui_component(countdownText);
        countdownText = NULL;
    }
    
    if (this->pauseMenu) {
        pause_menu_destroy(this->pauseMenu);
        this->pauseMenu = NULL;
    }

    if (this->finishRaceMenu) {
        finish_race_menu_destroy(this->finishRaceMenu);
        this->finishRaceMenu = NULL;
    }

    if (this->nameInputContainer) {
        destroy_ui_component(this->nameInputContainer);
        this->nameInputContainer = NULL;
        this->nameInputText = NULL;
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
    this->cronometer_time = 0.0f;
    this->replay_requested = false;  

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
    if (road_load(&this->road_data, road_data_file, 1200, 0x8EC940, road_surface_file, loading_ui) != 0) {
        printf("Failed to load road data\n");
        base_destroy(&this->base);
        free(this);
        return NULL;
    }

    // Initialize Player
    int creating_car_index = 0;
    Point player_start_pos = road_get_start_point(&this->road_data, creating_car_index);
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

        Point ai_start_pos = road_get_start_point(&this->road_data, creating_car_index);
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
            fprintf(stderr, "game_state_create_playing: Failed to create AI car %d.\n", i);
            for (int j = 0; j < i; j++) {
                ai_car_destroy(this->ai_cars[j]);
            }
        }
        ai_xpm_idx++;
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
    this->finish_race_delay_timer = 0.0f;
    this->player_skid_input_active = false;
    this->player_turn_input_sign = 0;
    this->total_laps = MAX_LAPS;
    this->current_lap = 0;
    this->race_timer_s = 0.0f;
    this->player_has_finished = false;
    this->player_finish_time = 0.0f;
    this->pause_requested = false;
    this->pauseMenu = NULL;
    this->finishRaceMenu = NULL;
    this->current_total_racers = 0;
    memset(this->current_race_positions, 0, sizeof(this->current_race_positions));

    // Initialize wrong direction detection
    this->previous_player_score = 0;
    this->wrong_direction_timer = 0.0f;
    this->is_going_wrong_direction = false;

    this->timer_count_down = 3.99f;


    countdownText = create_text_component("3", gameFont, 0xFFFFFF); 
    if (countdownText && countdownText->data) {
        TextElementData *data = (TextElementData *)countdownText->data;
        countdownText->x = vbe_mode_info.XResolution / 2 - data->width / 2;
        countdownText->y = vbe_mode_info.YResolution / 2 - data->height / 2;
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

bool playing_is_replay_requested(Game *this) {
    return this->replay_requested;
}

static void create_name_input_ui(Game *this) {
    if (this->nameInputContainer) {
        destroy_ui_component(this->nameInputContainer);
        this->nameInputContainer = NULL;
        this->nameInputText = NULL;
    }

    memset(this->player_name, 0, MAX_NAME_LENGTH);
    this->name_length = 0;

    this->nameInputContainer = create_main_container(NULL, 30, 0, 0, 0, 0);
    set_container_background_color(this->nameInputContainer, 0x80000000);

    create_title_text("ENTER YOUR NAME", gameFont, 0xFFFFFF, this->nameInputContainer);

    UIComponent *instructionText = create_text_component("Type your name (max 9 chars) and press Enter", gameFont, 0xAAAAA);
    if (instructionText) {
        add_child_to_container_component(this->nameInputContainer, instructionText);
    }

    UIComponent *escText = create_text_component("Press ESC to skip", gameFont, 0x888888);
    if (escText) {
        add_child_to_container_component(this->nameInputContainer, escText);
    }

    UIComponent *inputContainer = create_container_component(0, 0, 400, 60);
    set_container_layout(inputContainer, LAYOUT_ROW, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(inputContainer, 0x2C2C2C);
    set_container_padding(inputContainer, 15, 15, 15, 15);
    set_container_border(inputContainer, 2, 0xFFDD00);
    set_container_border_radius(inputContainer, 8);

    char display_name[MAX_NAME_LENGTH + 2]; 
    sprintf(display_name, "%s_", this->player_name);
    this->nameInputText = create_text_component(display_name, gameFont, 0xFFFFFF);
    if (this->nameInputText) {
        add_child_to_container_component(inputContainer, this->nameInputText);
    }

    add_child_to_container_component(this->nameInputContainer, inputContainer);

    char time_str[32];
    int minutes = (int)(this->player_finish_time / 60);
    int seconds = (int)(this->player_finish_time) % 60;
    int milliseconds = (int)((this->player_finish_time - (int)this->player_finish_time) * 100);
    sprintf(time_str, "Your time: %02d:%02d.%02d", minutes, seconds, milliseconds);
    
    UIComponent *timeText = create_text_component(time_str, gameFont, 0x00FF00);
    if (timeText) {
        add_child_to_container_component(this->nameInputContainer, timeText);
    }

    perform_container_layout(this->nameInputContainer);
}

static void update_name_input_display(Game *this) {
    if (!this->nameInputText) return;

    char display_name[MAX_NAME_LENGTH + 2]; 
    sprintf(display_name, "%s_", this->player_name);

    if (this->nameInputText->data) {
        TextElementData *data = (TextElementData *)this->nameInputText->data;
        if (data->pixel_data) {
            free(data->pixel_data);
        }

        int text_width = 0, text_height = 0;
        for (int i = 0; display_name[i] != '\0'; i++) {
            GlyphData glyphData;
            if (font_get_glyph_data(gameFont, display_name[i], &glyphData)) {
                text_width += glyphData.xadvance;
                if (glyphData.height > text_height) {
                    text_height = glyphData.height;
                }
            }
        }
        
        if (text_width > 0 && text_height > 0) {
            uint32_t *text_buffer = malloc(text_width * text_height * sizeof(uint32_t));
            if (text_buffer) {
                for (int i = 0; i < text_width * text_height; i++) {
                    text_buffer[i] = 0x00000000;
                }
                
                if (load_text(display_name, 0, 0, 0xFFFFFF, gameFont, text_buffer, text_width) == 0) {
                    data->pixel_data = text_buffer;
                    data->width = text_width;
                    data->height = text_height;
                    
                    perform_container_layout(this->nameInputContainer);
                } else {
                    free(text_buffer);
                }
            }
        }
    }
}

