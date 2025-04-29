#include <lcom/lcf.h>
#include "menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t index_packet;
extern struct packet pp;

static Sprite *title_sprite = NULL;
static Sprite *play_sprite = NULL;
static Sprite *leaderboard_sprite = NULL;
static Sprite *quit_sprite = NULL;

static int x_center, y_center;

// Mouse cursor

void draw_mouse_pointer(int x, int y, bool is_hovering) {
    uint32_t color = is_hovering ? 0xFF0000 : 0xFFFFFF; 
    for (int dx = -4; dx <= 4; dx++) {
        vg_draw_pixel(x + dx, y, color); 
    }
    for (int dy = -4; dy <= 4; dy++) {
        vg_draw_pixel(x, y + dy, color); 
    }
}

void clear_mouse_pointer(int x, int y) {
    for (int dx = -4; dx <= 4; dx++) {
        vg_draw_pixel(x + dx, y, BACKGROUND_COLOR); 
    }
    for (int dy = -4; dy <= 4; dy++) {
        vg_draw_pixel(x, y + dy, BACKGROUND_COLOR); 
    }
}


// Main menu

void (draw_static_menu)() {
    draw_sprite_xpm(title_sprite, (vbe_mode_info.XResolution - title_sprite->width) / 2, y_center - 120);
    draw_sprite_xpm(play_sprite, x_center, y_center);
    draw_sprite_xpm(leaderboard_sprite, (vbe_mode_info.XResolution - leaderboard_sprite->width) / 2, y_center + 80);
    draw_sprite_xpm(quit_sprite, x_center, y_center + 170);
}

bool is_mouse_over_option(int mouse_x, int mouse_y, int *selected) {
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

void update_mouse_position(int *x, int *y) {
    *x += pp.delta_x;
    *y -= pp.delta_y;

    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= vbe_mode_info.XResolution) *x = vbe_mode_info.XResolution - 1;
    if (*y >= vbe_mode_info.YResolution) *y = vbe_mode_info.YResolution - 1;
}

bool handle_mouse_input(int *mouse_x, int *mouse_y, int *selected_option) {
    clear_mouse_pointer(*mouse_x, *mouse_y); 
    update_mouse_position(mouse_x, mouse_y); 

    draw_static_menu(); 
    bool is_hovering = is_mouse_over_option(*mouse_x, *mouse_y, selected_option);
    draw_mouse_pointer(*mouse_x, *mouse_y, is_hovering);

    return pp.lb && *selected_option != -1;
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


// Game over

int (draw_game_over_screen)() {

    return 0;
}


