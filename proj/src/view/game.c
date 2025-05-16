#include <lcom/lcf.h>

#include "game.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;

static void game_draw_internal(GameState *base) {
    Game *this = (Game *)base;

    draw_road_background(this->road_sprite1, this->road_sprite2, this->road_y1, this->road_y2);

    if (this->playerCar.car_sprite) {
        sprite_draw_xpm(this->playerCar.car_sprite, this->playerCar.x, this->playerCar.y, true);
    }
}

static void game_process(GameState *base, EventType event) {
    Game *this = (Game *)base;
    if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case ESC_BREAKCODE:
                this->currentSubstate = GAME_EXITED;
                break;
            case LEFT_ARROW:
                if (this->playerCar.x > 0) {
                    car_move_left(&this->playerCar, LEFT_BOUNDARY, STEP);
                }
                break;
            case RIGHT_ARROW:
                if (this->playerCar.x > 0) {
                    car_move_right(&this->playerCar, RIGHT_BOUNDARY, STEP);
                }
                break;
            default:
                break;
        }
        base->draw(base);
    } else if (event == EVENT_MOUSE) {
        base->draw(base);
    }
}

static void game_update(GameState *base) {
    Game *this = (Game *)base;

    this->road_y1 += 2;
    this->road_y2 += 2;

    if (this->road_y1 >= (int)vbe_mode_info.YResolution)
        this->road_y1 = -this->road_sprite1->height;

    if (this->road_y2 >= (int)vbe_mode_info.YResolution)
        this->road_y2 = -this->road_sprite2->height;
}

static void game_destroy_internal(GameState *base) {
    Game *this = (Game *)base;
    if (this) {
        sprite_destroy(this->playerCar.car_sprite);
        sprite_destroy(this->road_sprite1);
        sprite_destroy(this->road_sprite2);
    }
    free(base);
}

Game *game_create(int car_choice) {
    Game *this = (Game *) malloc(sizeof(Game));
    if (this == NULL) {
        return NULL;
    }

    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))game_draw_internal;
    this->base.process_event = game_process;
    this->base.update_state = game_update;
    this->base.destroy = game_destroy_internal;

    this->currentSubstate = GAME_SUBSTATE_PLAYING;

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

    return this;
}


void game_destroy(Game *this) {
    this->base.destroy(&this->base);
}

void game_draw(Game *this) {
    this->base.draw(&this->base);
}

void game_process_event(Game *this, EventType event) {
    this->base.process_event(&this->base, event);
}

void game_update_state(Game *this) {
    this->base.update_state(&this->base);
}

GameSubstate game_get_current_substate(Game *this) {
    return this->currentSubstate;
}

void game_reset_state(Game *this) {
    this->currentSubstate = GAME_SUBSTATE_PLAYING;
    cleanup_road_background();
}

