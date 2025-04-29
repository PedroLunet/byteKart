#include <lcom/lcf.h>

#include "menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;

// MAIN SCREEN

void (draw_static_menu)(Sprite *title_sprite, Sprite *play_sprite, Sprite *leaderboard_sprite, Sprite *quit_sprite, uint32_t y_center) {
    draw_sprite_xpm(title_sprite, (vbe_mode_info.XResolution - title_sprite->width) / 2, y_center - 120);
    draw_sprite_xpm(play_sprite, (vbe_mode_info.XResolution - play_sprite->width) / 2, y_center);
    draw_sprite_xpm(leaderboard_sprite, (vbe_mode_info.XResolution - leaderboard_sprite->width) / 2, y_center + 80);
    draw_sprite_xpm(quit_sprite, (vbe_mode_info.XResolution - quit_sprite->width) / 2, y_center + 170);
}

void (select_main_menu_option)(int option, Sprite *title_sprite, Sprite *play_sprite, Sprite *leaderboard_sprite, Sprite *quit_sprite, uint32_t x_center, uint32_t y_center) {
    vg_draw_rectangle(x_center - 10, y_center - 10, play_sprite->width + 20, play_sprite->height + 20, 0x000000); 
    vg_draw_rectangle(((vbe_mode_info.XResolution - leaderboard_sprite->width) / 2) - 10, y_center + 70, leaderboard_sprite->width + 20, leaderboard_sprite->height + 20, 0x000000); 
    vg_draw_rectangle(x_center - 10, y_center + 170, quit_sprite->width + 20, quit_sprite->height + 20, 0x000000); 

    if (option == 0) {
        vg_draw_rectangle(x_center - 10, y_center - 10, play_sprite->width + 20, play_sprite->height + 20, 0xFFFFFF);
    } else if (option == 1) { 
        vg_draw_rectangle(((vbe_mode_info.XResolution - leaderboard_sprite->width) / 2) - 10, y_center + 70, leaderboard_sprite->width + 20, leaderboard_sprite->height + 20, 0xFFFFFF);
    } else if (option == 2) { 
        vg_draw_rectangle(x_center - 10, y_center + 170, quit_sprite->width + 20, quit_sprite->height + 20, 0xFFFFFF);
    }

    draw_sprite_xpm(play_sprite, x_center, y_center);
    draw_sprite_xpm(leaderboard_sprite, (vbe_mode_info.XResolution - leaderboard_sprite->width) / 2, y_center + 80);
    draw_sprite_xpm(quit_sprite, x_center, y_center + 170);
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
    uint8_t irq_set_keyboard;

    if (kbc_subscribe_int(&irq_set_keyboard) != 0) {
        return 1;
    }

    int selected_option = 0; 
    bool exit_menu = false;

    while ( !exit_menu ) {
        /* Get a request message. */
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
          printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) { /* received notification */
          switch (_ENDPOINT_P(msg.m_source)) {
            case HARDWARE: /* hardware interrupt notification */
              if (msg.m_notify.interrupts & BIT(irq_set_keyboard)) { /* subscribed interrupt */
                kbc_ih();
                switch (scancode) {
                    case UP_ARROW: 
                        if (selected_option > 0) selected_option--;
                        draw_main_screen(selected_option);
                        break;
                    case DOWN_ARROW: 
                        if (selected_option < 2) selected_option++;
                        draw_main_screen(selected_option);
                        break;
                    case ENTER_KEY: 
                        exit_menu = true;
                        break;
                    case ESC_BREAKCODE: 
                        exit_menu = true;
                        selected_option = -1; // Exit without selecting
                        break;
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
    
    if (kbc_unsubscribe_int() != 0) {
        return 1;
    }

    return selected_option; 
}


// GAME OVER SCREEN

int (draw_game_over_screen)() {

    return 0;
}


