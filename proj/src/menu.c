#include <lcom/lcf.h>
#include <stdlib.h>

#include "menu.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "controller/video_card.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;

static void drawStaticMenu(Menu *this) {
    sprite_draw_xpm(this->titleSprite, (vbe_mode_info.YResolution - this->titleSprite->width) / 2, this->centerY - 120);
    sprite_draw_xpm(this->playSprite, this->centerX, this->centerY);
    sprite_draw_xpm(this->leaderboardSprite, (vbe_mode_info.YResolution - this->leaderboardSprite->width) / 2, this->centerY + 80);
    sprite_draw_xpm(this->quitSprite, this->centerX, this->centerY + 170);
}

static void selectMainMenuOption(Menu *this) {

    vg_draw_rectangle(this->centerX - 10, this->centerY - 10, this->playSprite->width + 20, this->playSprite->height + 20, 0x000000);
    vg_draw_rectangle(((vbe_mode_info.YResolution - this->leaderboardSprite->width) / 2) - 10, this->centerY + 70, this->leaderboardSprite->width + 20, this->leaderboardSprite->height + 20, 0x000000);
    vg_draw_rectangle(this->centerX - 10, this->centerY + 170, this->quitSprite->width + 20, this->quitSprite->height + 20, 0x000000);

    if (this->selectedOption == 0) {
        vg_draw_rectangle(this->centerX - 10, this->centerY - 10, this->playSprite->width + 20, this->playSprite->height + 20, 0xFFFFFF);
    } else if (this->selectedOption == 1) {
        vg_draw_rectangle(((vbe_mode_info.YResolution - this->leaderboardSprite->width) / 2) - 10, this->centerY + 70, this->leaderboardSprite->width + 20, this->leaderboardSprite->height + 20, 0xFFFFFF);
    } else if (this->selectedOption == 2) {
        vg_draw_rectangle(this->centerX - 10, this->centerY + 170, this->quitSprite->width + 20, this->quitSprite->height + 20, 0xFFFFFF);
    }

    sprite_draw_xpm(this->playSprite, this->centerX, this->centerY);
    sprite_draw_xpm(this->leaderboardSprite, (vbe_mode_info.YResolution - this->leaderboardSprite->width) / 2, this->centerY + 80);
    sprite_draw_xpm(this->quitSprite, this->centerX, this->centerY + 170);
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
    this->currentSubstate = MENU_MAIN;
    this->selectedOption = 0;

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


