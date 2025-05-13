#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include "macros.h"
#include "sprite.h"

struct GameState;

typedef struct GameState {
    int mouse_x;
    int mouse_y;
    int prev_mouse_x;
    int prev_mouse_y;
    uint32_t centerX;
    uint32_t centerY;
    Sprite *cursorSprite;
    Sprite *cursorPointerSprite;
    Sprite *prev_cursor;
    bool is_hovering;
    bool mouse_dirty;
    int prev_cursor_width;
    int prev_cursor_height;
    int mouse_displacement_x;
    int mouse_displacement_y;

    void (*draw)(struct GameState *this);
    void (*process_event)(struct GameState *this, EventType event);
    void (*update_state)(struct GameState *this);
    void (*destroy)(struct GameState *this);
    bool (*handle_mouse_input)(struct GameState *this, void (*draw_state)(struct GameState *), bool (*is_over)(struct GameState *, int, int, void *), void *hover_target);
    bool (*is_mouse_over)(struct GameState *this, int x, int y, void *target);
    void (*update_mouse_position)(struct GameState *this, int *x, int *y);
    void (*draw_mouse)(struct GameState *this);
    void (*clear_mouse_area)(struct GameState *this);
    void (*update_mouse_delta)(struct GameState *this);
    void (*reset_mouse_delta)(struct GameState *this);
} GameState;

void base_destroy(GameState *this);
void init_base_game_state(GameState *state);

#endif //GAME_STATE_H
