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

    void (*draw)(struct GameState *this);
    void (*process_event)(struct GameState *this, EventType event);
    void (*update_state)(struct GameState *this);
    void (*destroy)(struct GameState *this);
    bool (*handle_mouse_input)(struct GameState *this, void (*draw_state)(struct GameState *), bool (*is_over)(struct GameState *, int, int, void *), void *hover_target);
    bool (*is_mouse_over)(struct GameState *this, int x, int y, void *target);
    void (*update_mouse_position)(struct GameState *this, int *x, int *y);
} GameState;


void base_destroy(GameState *this);
void init_base_game_state(GameState *state);

#endif //GAME_STATE_H
