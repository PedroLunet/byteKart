#include <lcom/lcf.h>
#include <stdlib.h>

#include "game_state.h"
#include "controller/video_card.h"
#include "controller/mouse.h"
#include "xpm/xpm_files.h"

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

static void base_clear_mouse_area(GameState *this) {
    if (this->mouse_dirty) {
        int prev_x = this->prev_mouse_x;
        int prev_y = this->prev_mouse_y;
        int width = this->prev_cursor_width;
        int height = this->prev_cursor_height;
        uint32_t clear_color = BACKGROUND_COLOR;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int draw_x = prev_x + x;
                int draw_y = prev_y + y;
                if (draw_x >= 0 && draw_x < (int)vbe_mode_info.XResolution &&
                    draw_y >= 0 && draw_y < (int)vbe_mode_info.YResolution) {
                    vg_draw_pixel(draw_x, draw_y, clear_color);
                }
            }
        }
    }
}

static void base_draw_mouse(GameState *this) {
     if (this->mouse_dirty) {
        Sprite *sprite_to_draw = this->is_hovering && this->cursorPointerSprite ? this->cursorPointerSprite : this->cursorSprite;
        if (sprite_to_draw) {
            sprite_draw_xpm(sprite_to_draw, this->mouse_x, this->mouse_y, true);
        }
        this->mouse_dirty = false;
    }
}

static bool base_handle_mouse_input(GameState *this, void (*draw_state)(GameState *), bool (*is_over)(GameState *, int, int, void *), void *hover_target) {
    bool mouse_moved = (this->mouse_x != this->prev_mouse_x) || (this->mouse_y != this->prev_mouse_y);

    if (mouse_moved) {
        this->prev_cursor_width = (this->cursorSprite) ? this->cursorSprite->width : (this->cursorPointerSprite ? this->cursorPointerSprite->width : 0);
        this->prev_cursor_height = (this->cursorSprite) ? this->cursorSprite->height : (this->cursorPointerSprite ? this->cursorPointerSprite->height : 0);
        this->prev_mouse_x = this->mouse_x;
        this->prev_mouse_y = this->mouse_y;
        this->mouse_dirty = true;
    }

    base_update_mouse_position(this, &this->mouse_x, &this->mouse_y);

    if (draw_state) {
        draw_state(this);
    }

    this->is_hovering = (is_over && hover_target) ? is_over(this, this->mouse_x, this->mouse_y, hover_target) : false;

    return pp.lb && this->is_hovering;
}

static void update_mouse_delta(GameState *this) {
    this->mouse_displacement_x += pp.delta_x;
    this->mouse_displacement_y += pp.delta_y;
}

static void reset_mouse_delta(GameState *this) {
    this->mouse_displacement_x = 0;
    this->mouse_displacement_y = 0;
}

void base_destroy(GameState *this) {
    if (this) {
        sprite_destroy(this->cursorSprite);
        sprite_destroy(this->cursorPointerSprite);
        free(this);
    }
}

void init_base_game_state(GameState *state) {
    state->mouse_x = (int)vbe_mode_info.XResolution / 2;
    state->mouse_y = (int)vbe_mode_info.YResolution / 2;
    state->centerX = (vbe_mode_info.XResolution) / 2;
    state->centerY = (vbe_mode_info.YResolution) / 2;
    state->prev_mouse_x = state->mouse_x;
    state->prev_mouse_y = state->mouse_y;

    state->draw = NULL; // To be overridden
    state->process_event = NULL; // To be overridden
    state->update_state = NULL; // To be overridden
    state->destroy = base_destroy;
    state->handle_mouse_input = base_handle_mouse_input;
    state->is_mouse_over = NULL; // To be overridden if needed
    state->update_mouse_position = base_update_mouse_position;
    state->draw_mouse = base_draw_mouse;
    state->clear_mouse_area = base_clear_mouse_area;
    state->update_mouse_delta = update_mouse_delta;
    state->reset_mouse_delta = reset_mouse_delta;

    state->is_hovering = false;
    state->cursorSprite = sprite_create_xpm((xpm_map_t) cursor, 0, 0, 0, 0);
    state->cursorPointerSprite = sprite_create_xpm((xpm_map_t) cursor_pointer, 0, 0, 0, 0);
    state->prev_cursor = state->cursorSprite;
    state->mouse_dirty = false;
    state->prev_cursor_width = 0;
    state->prev_cursor_height = 0;
    state->mouse_displacement_x = 0;
    state->mouse_displacement_y = 0;
}

