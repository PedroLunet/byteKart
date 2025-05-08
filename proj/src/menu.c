#include <lcom/lcf.h>

#include "menu.h"
#include "xpm/xpm_files.h"
#include "macros.h"
#include "controller/video_card.h"
#include "controller/mouse.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;

static void drawStaticMenu(Menu *this) {
    sprite_draw_xpm(this->titleSprite, (vbe_mode_info.XResolution - this->titleSprite->width) / 2, this->base.centerY - 120);
    sprite_draw_xpm(this->playSprite, this->base.centerX, this->base.centerY);
    sprite_draw_xpm(this->leaderboardSprite, (vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2, this->base.centerY + 80);
    sprite_draw_xpm(this->quitSprite, this->base.centerX, this->base.centerY + 170);
}

static void selectMainMenuOption(Menu *this) {
    vg_draw_rectangle(this->base.centerX - 10, this->base.centerY - 10, this->playSprite->width + 20, this->playSprite->height + 20, 0x000000);
    vg_draw_rectangle(((vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2) - 10, this->base.centerY + 70, this->leaderboardSprite->width + 20, this->leaderboardSprite->height + 20, 0x000000);
    vg_draw_rectangle(this->base.centerX - 10, this->base.centerY + 170, this->quitSprite->width + 20, this->quitSprite->height + 20, 0x000000);

    if (this->selectedOption == 0) {
        vg_draw_rectangle(this->base.centerX - 10, this->base.centerY - 10, this->playSprite->width + 20, this->playSprite->height + 20, 0xFFFFFF);
    } else if (this->selectedOption == 1) {
        vg_draw_rectangle(((vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2) - 10, this->base.centerY + 70, this->leaderboardSprite->width + 20, this->leaderboardSprite->height + 20, 0xFFFFFF);
    } else if (this->selectedOption == 2) {
        vg_draw_rectangle(this->base.centerX - 10, this->base.centerY + 170, this->quitSprite->width + 20, this->quitSprite->height + 20, 0xFFFFFF);
    }

    sprite_draw_xpm(this->playSprite, this->base.centerX, this->base.centerY);
    sprite_draw_xpm(this->leaderboardSprite, (vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2, this->base.centerY + 80);
    sprite_draw_xpm(this->quitSprite, this->base.centerX, this->base.centerY + 170);
}

// --- Internal Menu Drawing Functions ---
static void menu_draw_internal(GameState *base) {
    Menu *this = (Menu *)base;
    drawStaticMenu(this);
    selectMainMenuOption(this);
}

// --- Internal Menu Mouse Handling ---
static bool menu_is_mouse_over_option(GameState *base, int mouse_x, int mouse_y, void *data) {
    Menu *this = (Menu *)base;
    int *selected_option = (int *)data;
    if (mouse_x >= (int)this->base.centerX && mouse_x <= (int)this->base.centerX + (int)this->playSprite->width &&
        mouse_y >= (int)this->base.centerY && mouse_y <= (int)this->base.centerY + (int)this->playSprite->height) {
        *selected_option = 0;
        return true;
    }

    int leaderboard_x = (int)(vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2;
    if (mouse_x >= leaderboard_x && mouse_x <= leaderboard_x + (int)this->leaderboardSprite->width &&
        mouse_y >= (int)this->base.centerY + 80 && mouse_y <= (int)this->base.centerY + 80 + (int)this->leaderboardSprite->height) {
        *selected_option = 1;
        return true;
    }

    if (mouse_x >= (int)this->base.centerX && mouse_x <= (int)this->base.centerX + (int)this->quitSprite->width &&
        mouse_y >= (int)this->base.centerY + 170 && mouse_y <= (int)this->base.centerY + 170 + (int)this->quitSprite->height) {
        *selected_option = 2;
        return true;
    }

    *selected_option = -1;
    return false;
}

// --- Internal Menu Event Processing ---
static void menu_process(GameState *base, EventType event) {
    Menu *this = (Menu *)base;
    if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case UP_ARROW:
                if (this->selectedOption > 0) this->selectedOption--;
                break;
            case DOWN_ARROW:
                if (this->selectedOption < 2) this->selectedOption++;
                break;
            case ENTER_KEY:
                if (this->selectedOption == 0) {
                    this->currentSubstate = MENU_FINISHED_PLAY;
                } else if (this->selectedOption == 2) {
                    this->currentSubstate = MENU_FINISHED_QUIT;
                }
                break;
            case ESC_BREAKCODE:
                this->currentSubstate = MENU_EXITED;
                break;
            default:
                break;
        }
        base->draw(base); // Redraw the menu
    } else if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))menu_draw_internal, menu_is_mouse_over_option, &this->selectedOption)) {
            if (this->selectedOption == 0) {
                this->currentSubstate = MENU_FINISHED_PLAY;
            } else if (this->selectedOption == 2) {
                this->currentSubstate = MENU_FINISHED_QUIT;
            }
        }
    }
}

// --- Internal Menu Destroy Function ---
static void menu_destroy_internal(GameState *base) {
    Menu *this = (Menu *)base;
    if (this) {
        sprite_destroy(this->titleSprite);
        sprite_destroy(this->playSprite);
        sprite_destroy(this->leaderboardSprite);
        sprite_destroy(this->quitSprite);
        free(this);
    }
}

Menu *menu_create() {
    Menu *this = (Menu *) malloc(sizeof(Menu));
    if (this == NULL) {
        return NULL;
    }

    // Initialize the base GameState
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))menu_draw_internal;
    this->base.process_event = menu_process;
    this->base.destroy = menu_destroy_internal;
    this->base.is_mouse_over = menu_is_mouse_over_option;

    // Initialize menu-specific data
    this->titleSprite = sprite_create_xpm((xpm_map_t) title, 0, 0, 0, 0);
    if (!this->titleSprite) {
        free(this);
        return NULL;
    }

    this->playSprite = sprite_create_xpm((xpm_map_t) play, 0, 0, 0, 0);
    if (!this->playSprite) {
        sprite_destroy(this->titleSprite);
        free(this);
        return NULL;
    }

    this->leaderboardSprite = sprite_create_xpm((xpm_map_t) leaderboard, 0, 0, 0, 0);
    if (!this->leaderboardSprite) {
        sprite_destroy(this->titleSprite);
        sprite_destroy(this->playSprite);
        free(this);
        return NULL;
    }

    this->quitSprite = sprite_create_xpm((xpm_map_t) quit, 0, 0, 0, 0);
    if (!this->quitSprite) {
        sprite_destroy(this->titleSprite);
        sprite_destroy(this->playSprite);
        sprite_destroy(this->leaderboardSprite);
        free(this);
        return NULL;
    }

    this->selectedOption = 0;
    this->currentSubstate = MENU_MAIN;

    return this;
}

void menu_destroy(Menu *this) {
    this->base.destroy(&this->base);
}

void menu_draw(Menu *this) {
    this->base.draw(&this->base);
}

void menu_process_event(Menu *this, EventType event) {
    this->base.process_event(&this->base, event);
}

MenuSubstate menu_get_current_substate(Menu *this) {
    return this->currentSubstate;
}

void menu_reset_state(Menu *this) {
    this->currentSubstate = MENU_MAIN;
    this->selectedOption = 0;
    this->base.draw(&this->base);
}

