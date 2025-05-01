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

static void draw_mouse_pointer(int x, int y, bool is_hovering) {
    uint32_t color = is_hovering ? 0xFF0000 : 0xFFFFFF;
    for (int dx = -4; dx <= 4; dx++) {
        vg_draw_pixel(x + dx, y, color);
    }
    for (int dy = -4; dy <= 4; dy++) {
        vg_draw_pixel(x, y + dy, color);
    }
}

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

static bool is_mouse_over_option(int mouse_x, int mouse_y, int *selected) {
    if (mouse_x >= x_center && mouse_x <= x_center + play_sprite->width &&
        mouse_y >= y_center && mouse_y <= y_center + play_sprite->height) {
        *selected = 0;
        return true;
        }

    int leaderboard_x = (vbe_mode_info.XResolution - leaderboard_sprite->width) / 2;
    if (mouse_x >= leaderboard_x && mouse_x <= leaderboard_x + leaderboard_sprite->width &&
        mouse_y >= y_center + 80 && mouse_y <= y_center + 80 + leaderboard_sprite->height) {
        *selected = 1;
        return true;
        }

    if (mouse_x >= x_center && mouse_x <= x_center + quit_sprite->width &&
        mouse_y >= y_center + 170 && mouse_y <= y_center + 170 + quit_sprite->height) {
        *selected = 2;
        return true;
        }

    *selected = -1;
    return false;
}

static void update_mouse_position(int *x, int *y) {
    *x += pp.delta_x;
    *y -= pp.delta_y;

    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= vbe_mode_info.XResolution) *x = vbe_mode_info.XResolution - 1;
    if (*y >= vbe_mode_info.YResolution) *y = vbe_mode_info.YResolution - 1;
}

static bool handle_mouse_input(int *mouse_x, int *mouse_y, int *selected_option) {
    clear_mouse_pointer(*mouse_x, *mouse_y);
    update_mouse_position(mouse_x, mouse_y);

    draw_static_menu();
    bool is_hovering = is_mouse_over_option(*mouse_x, *mouse_y, selected_option);
    draw_mouse_pointer(*mouse_x, *mouse_y, is_hovering);

    return pp.lb && *selected_option != -1;
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

int (draw_main_screen)(int option) {

    Sprite *title_sprite = create_sprite_xpm((xpm_map_t) title, 0, 0, 0, 0);
    if (title_sprite == NULL) return 1; 

    Sprite *play_sprite = create_sprite_xpm((xpm_map_t) play, 0, 0, 0, 0);
    if (play_sprite == NULL) return 1;

    Sprite *leaderboard_sprite = create_sprite_xpm((xpm_map_t) leaderboard, 0, 0, 0, 0);

    Sprite *quit_sprite = create_sprite_xpm((xpm_map_t) quit, 0, 0, 0, 0);
    if (quit_sprite == NULL) return 1;

    uint32_t x_center = (vbe_mode_info.XResolution - play_sprite->width) / 2;
    uint32_t y_center = (vbe_mode_info.YResolution - play_sprite->height) / 2;

    draw_static_menu(title_sprite, play_sprite, leaderboard_sprite, quit_sprite, y_center);
    select_main_menu_option(option, title_sprite, play_sprite, leaderboard_sprite, quit_sprite, x_center, y_center);

    return 0;
}

int (navigate_main_menu)() {
    int ipc_status, r;
    message msg;
    uint8_t irq_set_mouse;

    if (mouse_subscribe_int(&irq_set_mouse) != 0) return 1;
    if (mouse_write_command(MOUSE_ENABLE) != 0) return 1;

    title_sprite = create_sprite_xpm((xpm_map_t) title, 0, 0, 0, 0);
    play_sprite = create_sprite_xpm((xpm_map_t) play, 0, 0, 0, 0);
    leaderboard_sprite = create_sprite_xpm((xpm_map_t) leaderboard, 0, 0, 0, 0);
    quit_sprite = create_sprite_xpm((xpm_map_t) quit, 0, 0, 0, 0);
    if (!title_sprite || !play_sprite || !leaderboard_sprite || !quit_sprite) return 1;

    x_center = (vbe_mode_info.XResolution - play_sprite->width) / 2;
    y_center = (vbe_mode_info.YResolution - play_sprite->height) / 2;

    int mouse_x = vbe_mode_info.XResolution / 2;
    int mouse_y = vbe_mode_info.YResolution / 2;
    int selected_option = -1;
    bool done = false;

    draw_static_menu();
    draw_mouse_pointer(mouse_x, mouse_y, false);

    while ( !done ) {
        /* Get a request message. */
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
          printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) { /* received notification */
          switch (_ENDPOINT_P(msg.m_source)) {
            case HARDWARE: /* hardware interrupt notification */
              if (msg.m_notify.interrupts & BIT(irq_set_mouse)) { /* subscribed interrupt */
                mouse_ih();
                mouse_bytes();
                if (index_packet == 3) {
                    mouse_struct_packet(&pp);
                    index_packet = 0;
                    done = handle_mouse_input(&mouse_x, &mouse_y, &selected_option);
                }
              }
              break;
            default:
              break; /* no other notifications expected: do nothing */
          }
        } else { /* received a standard message, not a notification */
            /* no standard messages expected: do nothing */
        }
    }

    if (mouse_write_command(MOUSE_DISABLE) != 0) return 1;
    if (mouse_unsubscribe_int() != 0) return 1;

    return selected_option;
}

MenuSubstate menu_get_current_substate(Menu *this) {
    return this->currentSubstate;
}

void menu_reset_state(Menu *this) {
    this->currentSubstate = MENU_MAIN;
    this->selectedOption = 0;
    menu_draw(this);


