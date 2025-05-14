#include <lcom/lcf.h>

#include "game.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;

static void drawRoadBackground(Game *this) {
    draw_road_background();
}

static void game_draw_internal(GameState *base) {
    Game *this = (Game *)base;
    drawRoadBackground(this);

    if (this->playerCar.car_sprite) {
        sprite_draw_xpm(this->playerCar.car_sprite, this->playerCar.x, this->playerCar.y);
    }
}

static void game_process(GameState *base, EventType event) {
    Game *this = (Game *)base;
    if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case ESC_BREAKCODE:
                this->currentSubstate = GAME_EXITED;
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
    if (this->road_y1 >= (int)vbe_mode_info.YResolution) this->road_y1 = -this->road_sprite1->height;
    if (this->road_y2 >= (int)vbe_mode_info.YResolution) this->road_y2 = -this->road_sprite2->height;
}

static void game_destroy_internal(GameState *base) {
    Game *this = (Game *)base;
    if (this) {
        sprite_destroy(this->playerCar.car_sprite);
    }
    free(base);
}

Game *game_create() {
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
    this->playerCar.car_sprite = sprite_create_xpm((xpm_map_t) pink_car_xpm, 0, 0, 0, 0);
    if (!this->playerCar.car_sprite) {
        free(this);
        return NULL;
    }

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

