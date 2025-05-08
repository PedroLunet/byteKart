#include <lcom/lcf.h>
#include <stdlib.h>

#include "game_state.h"
#include "controller/video_card.h"
#include "controller/mouse.h"

extern vbe_mode_info_t vbe_mode_info;
extern struct packet pp;

static void base_update_mouse_position(GameState *this, int *x, int *y) {
    *x += pp.delta_x;
    *y -= pp.delta_y;

    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= (int)vbe_mode_info.XResolution) *x = (int)vbe_mode_info.XResolution - 1;
    if (*y >= (int)vbe_mode_info.YResolution) *y = (int)vbe_mode_info.YResolution - 1;
}

static void base_clear_mouse_pointer(GameState *this) {
    for (int dx = -4; dx <= 4; dx++) {
        vg_draw_pixel(this->mouse_x + dx, this->mouse_y, BACKGROUND_COLOR);
    }
    for (int dy = -4; dy <= 4; dy++) {
        vg_draw_pixel(this->mouse_x, this->mouse_y + dy, BACKGROUND_COLOR);
    }
}

static void base_draw_mouse_pointer(GameState *this, uint32_t hover_color, uint32_t default_color, bool is_hovering) {
    uint32_t color = is_hovering ? hover_color : default_color;
    for (int dx = -4; dx <= 4; dx++) {
        vg_draw_pixel(this->mouse_x + dx, this->mouse_y, color);
    }
    for (int dy = -4; dy <= 4; dy++) {
        vg_draw_pixel(this->mouse_x, this->mouse_y + dy, color);
    }
}

static bool base_handle_mouse_input(GameState *this, void (*draw_state)(GameState *), bool (*is_over)(GameState *, int, int, void *), void *hover_target) {
    base_clear_mouse_pointer(this);
    base_update_mouse_position(this, &this->mouse_x, &this->mouse_y);

    if (draw_state) {
        draw_state(this);
    }

    bool is_hovering = false;
    if (is_over && hover_target) {
        is_hovering = is_over(this, this->mouse_x, this->mouse_y, hover_target);
    }
    base_draw_mouse_pointer(this, 0xFF0000, 0xFFFFFF, is_hovering);

    return pp.lb && is_hovering;
}

void base_destroy(GameState *this) {
    if (this) {
        free(this);
    }
}

void init_base_game_state(GameState *state) {
    state->mouse_x = (int)vbe_mode_info.XResolution / 2;
    state->mouse_y = (int)vbe_mode_info.YResolution / 2;
    state->centerX = (vbe_mode_info.XResolution) / 2;
    state->centerY = (vbe_mode_info.YResolution) / 2;

    state->draw = NULL; // To be overridden
    state->process_event = NULL; // To be overridden
    state->update_state = NULL; // To be overridden
    state->destroy = base_destroy;
    state->handle_mouse_input = base_handle_mouse_input;
    state->is_mouse_over = NULL; // To be overridden if needed
    state->update_mouse_position = base_update_mouse_position;
}

