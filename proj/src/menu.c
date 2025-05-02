#include <lcom/lcf.h>
#include <stdlib.h>

#include "menu.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "controller/video_card.h"
#include "controller/mouse.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;

static void draw_mouse_pointer(Menu *this, bool is_hovering) {
    uint32_t color = is_hovering ? 0xFF0000 : 0xFFFFFF;
    for (int dx = -4; dx <= 4; dx++) {
        vg_draw_pixel(this->mouse_x + dx, this->mouse_y, color);
    }
    for (int dy = -4; dy <= 4; dy++) {
        vg_draw_pixel(this->mouse_x, this->mouse_y + dy, color);
    }
}

static void clear_mouse_pointer(Menu *this) {
    for (int dx = -4; dx <= 4; dx++) {
        vg_draw_pixel(this->mouse_x + dx, this->mouse_y, 0x000000);
    }
    for (int dy = -4; dy <= 4; dy++) {
        vg_draw_pixel(this->mouse_x, this->mouse_y + dy, 0x000000);
    }
}

static void drawStaticMenu(Menu *this) {
    sprite_draw_xpm(this->titleSprite, (vbe_mode_info.XResolution - this->titleSprite->width) / 2, this->centerY - 120);
    sprite_draw_xpm(this->playSprite, this->centerX, this->centerY);
    sprite_draw_xpm(this->leaderboardSprite, (vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2, this->centerY + 80);
    sprite_draw_xpm(this->quitSprite, this->centerX, this->centerY + 170);
}

static void selectMainMenuOption(Menu *this) {
    vg_draw_rectangle(this->centerX - 10, this->centerY - 10, this->playSprite->width + 20, this->playSprite->height + 20, 0x000000);
    vg_draw_rectangle(((vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2) - 10, this->centerY + 70, this->leaderboardSprite->width + 20, this->leaderboardSprite->height + 20, 0x000000);
    vg_draw_rectangle(this->centerX - 10, this->centerY + 170, this->quitSprite->width + 20, this->quitSprite->height + 20, 0x000000);

    if (this->selectedOption == 0) {
        vg_draw_rectangle(this->centerX - 10, this->centerY - 10, this->playSprite->width + 20, this->playSprite->height + 20, 0xFFFFFF);
    } else if (this->selectedOption == 1) {
        vg_draw_rectangle(((vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2) - 10, this->centerY + 70, this->leaderboardSprite->width + 20, this->leaderboardSprite->height + 20, 0xFFFFFF);
    } else if (this->selectedOption == 2) {
        vg_draw_rectangle(this->centerX - 10, this->centerY + 170, this->quitSprite->width + 20, this->quitSprite->height + 20, 0xFFFFFF);
    }

    sprite_draw_xpm(this->playSprite, this->centerX, this->centerY);
    sprite_draw_xpm(this->leaderboardSprite, (vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2, this->centerY + 80);
    sprite_draw_xpm(this->quitSprite, this->centerX, this->centerY + 170);
}

static bool is_mouse_over_option(Menu *this, int mouse_x, int mouse_y, int *selected) {
    if (mouse_x >= (int)this->centerX && mouse_x <= (int)this->centerX + (int)this->playSprite->width &&
        mouse_y >= (int)this->centerY && mouse_y <= (int)this->centerY + (int)this->playSprite->height) {
        *selected = 0;
        return true;
    }

    int leaderboard_x = (int)(vbe_mode_info.XResolution - this->leaderboardSprite->width) / 2;
    if (mouse_x >= leaderboard_x && mouse_x <= leaderboard_x + (int)this->leaderboardSprite->width &&
        mouse_y >= (int)this->centerY + 80 && mouse_y <= (int)this->centerY + 80 + (int)this->leaderboardSprite->height) {
        *selected = 1;
        return true;
    }

    if (mouse_x >= (int)this->centerX && mouse_x <= (int)this->centerX + (int)this->quitSprite->width &&
        mouse_y >= (int)this->centerY + 170 && mouse_y <= (int)this->centerY + 170 + (int)this->quitSprite->height) {
        *selected = 2;
        return true;
    }

    *selected = -1;
    return false;
}

static void update_mouse_position(Menu *this, int *x, int *y) {
    *x += pp.delta_x;
    *y -= pp.delta_y;

    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= (int)vbe_mode_info.XResolution) *x = (int)vbe_mode_info.XResolution - 1;
    if (*y >= (int)vbe_mode_info.YResolution) *y = (int)vbe_mode_info.YResolution - 1;
}

static bool handle_mouse_input(Menu *this) {
    clear_mouse_pointer(this);
    update_mouse_position(this, &this->mouse_x, &this->mouse_y);

    drawStaticMenu(this);
    bool is_hovering = is_mouse_over_option(this, this->mouse_x, this->mouse_y, &this->selectedOption);
    draw_mouse_pointer(this, is_hovering);

    return pp.lb && this->selectedOption != -1;
}

Menu *menu_create() {
    Menu *this = (Menu *) malloc(sizeof(Menu));
    if (this == NULL) {
        return NULL;
    }

    this->titleSprite = sprite_create_xpm((xpm_map_t) title, 0, 0, 0, 0);
    if (this->titleSprite == NULL) {
        free(this);
        return NULL;
    }

    this->playSprite = sprite_create_xpm((xpm_map_t) play, 0, 0, 0, 0);
    if (this->playSprite == NULL) {
        sprite_destroy(this->titleSprite);
        free(this);
        return NULL;
    }

    this->leaderboardSprite = sprite_create_xpm((xpm_map_t) leaderboard, 0, 0, 0, 0);
    if (this->leaderboardSprite == NULL) {
        sprite_destroy(this->titleSprite);
        sprite_destroy(this->playSprite);
        free(this);
        return NULL;
    }

    this->quitSprite = sprite_create_xpm((xpm_map_t) quit, 0, 0, 0, 0);
    if (this->quitSprite == NULL) {
        sprite_destroy(this->titleSprite);
        sprite_destroy(this->playSprite);
        sprite_destroy(this->leaderboardSprite);
        free(this);
        return NULL;
    }

    this->centerX = (vbe_mode_info.XResolution - this->playSprite->width) / 2;
    this->centerY = (vbe_mode_info.YResolution - this->playSprite->height) / 2;
    this->mouse_x = (int)vbe_mode_info.XResolution / 2;
    this->mouse_y = (int)vbe_mode_info.YResolution / 2;
    this->selectedOption = 0;
    this->currentSubstate = MENU_MAIN;

    return this;
}

void menu_destroy(Menu *this) {
    if (this) {
        sprite_destroy(this->titleSprite);
        sprite_destroy(this->playSprite);
        sprite_destroy(this->leaderboardSprite);
        sprite_destroy(this->quitSprite);
        free(this);
    }
}

void menu_draw(Menu *this) {
    drawStaticMenu(this);
    selectMainMenuOption(this);
    draw_mouse_pointer(this, false);
}

void menu_process_event(Menu *this, EventType event) {
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
        menu_draw(this);
    } else if (event == EVENT_MOUSE) {
        if (handle_mouse_input(this)) {
            if (this->selectedOption == 0) {
                this->currentSubstate = MENU_FINISHED_PLAY;
            } else if (this->selectedOption == 2) {
                this->currentSubstate = MENU_FINISHED_QUIT;
            }
        }
    }
}

MenuSubstate menu_get_current_substate(Menu *this) {
    return this->currentSubstate;
}

void menu_reset_state(Menu *this) {
    this->currentSubstate = MENU_MAIN;
    this->selectedOption = 0;
    menu_draw(this);
}


