#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "game.h"

extern vbe_mode_info_t vbe_mode_info;
GameState gameState = GAME_STATE_MENU;

extern uint8_t scancode;

static Car car;
static Sprite *road;

void game_init() {
}

void game_draw() {
    draw_road_background(); // estrada
}

void game_cleanup() {
    sprite_destroy(road);
}

