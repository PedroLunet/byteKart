#include <lcom/lcf.h>

#include "menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;
extern Font *gameFont;

static UIComponent *menuContainer = NULL;
static UIComponent *titleText = NULL;
static UIComponent *playText =  NULL;
static UIComponent *playContainer = NULL;
static UIComponent *leaderboardText = NULL;
static UIComponent *leaderboardContainer = NULL;
static UIComponent *quitText = NULL;
static UIComponent *quitContainer = NULL;

static void menu_draw_internal(GameState *base) {
    Menu *this = (Menu *)base;
    if (this->uiRoot) {
        draw_ui_component(this->uiRoot);
    }
}

static void menu_clean_dirty_mouse_internal(GameState *base) {
    Menu *this = (Menu *)base;
    if (this->uiRoot) {
        draw_dirty_area(this->uiRoot, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
}

static bool menu_is_mouse_over_option(GameState *base, int mouse_x, int mouse_y, void *data) {
    Menu *this = (Menu *)base;
    int *selected_option = (int *)data;
    *selected_option = -1;

    if (this->uiRoot && this->uiRoot->type == TYPE_CONTAINER) {
        ContainerData *rootData = (ContainerData *)
        this->uiRoot->data;

        if (rootData->num_children >= 4) {
            UIComponent *play = rootData->children[1];
            UIComponent *leaderboard = rootData->children[2];
            UIComponent *quit = rootData->children[3];

            if (play && play->type == TYPE_CONTAINER && play->data) {
                ContainerData *playData = (ContainerData *)play->data;
                if (mouse_x >= play->x && mouse_x < play->x + playData->width &&
                    mouse_y >= play->y && mouse_y < play->y + playData->height) {
                    *selected_option = 0;
                    set_container_background_color(playContainer, 0xA81D1D);
                    return true;
                }
            }
            if (leaderboard && leaderboard->type == TYPE_CONTAINER && leaderboard->data) {
                ContainerData *leaderboardData = (ContainerData *)leaderboard->data;
                if (mouse_x >= leaderboard->x && mouse_x < leaderboard->x + leaderboardData->width &&
                    mouse_y >= leaderboard->y && mouse_y < leaderboard->y + leaderboardData->height) {
                    *selected_option = 1;
                    set_container_background_color(leaderboardContainer, 0xA81D1D);
                    return true;
                }
            }

            if (quit && quit->type == TYPE_CONTAINER && quit->data) {
                ContainerData *quitData = (ContainerData *)quit->data;
                if (mouse_x >= quit->x && mouse_x < quit->x + quitData->width &&
                    mouse_y >= quit->y && mouse_y < quit->y + quitData->height) {
                    *selected_option = 2;
                    set_container_background_color(quitContainer, 0xA81D1D);
                    return true;
                }
            }

            if (*selected_option != 0) {
                set_container_background_color(playContainer, 0x111111);
            }
            if (*selected_option != 1) {
                set_container_background_color(leaderboardContainer, 0x111111);
            }
            if (*selected_option != 2) {
                set_container_background_color(quitContainer, 0x111111);
            }
        }
    }

    return false;
}

static void menu_process(GameState *base, EventType event) {
    Menu *this = (Menu *)base;
    int prevSelected = this->selectedOption;
    if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case UP_ARROW:
                this->selectedOption = (this->selectedOption - 1 + 3) % 3;
                break;
            case DOWN_ARROW:
                this->selectedOption = (this->selectedOption + 1) % 3;
                break;
            case ENTER_KEY:
                if (this->selectedOption == 0) {
                    this->currentSubstate = MENU_FINISHED_PLAY;
                } else if (this->selectedOption == 1) {
                    this->currentSubstate = MENU_MAIN;
                } else if (this->selectedOption == 2) {
                    this->currentSubstate = MENU_FINISHED_QUIT;
                }
                break;
            case ESC_BREAKCODE:
                this->currentSubstate = MENU_FINISHED_QUIT;
                break;
            default:
                break;
        }
    } else if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))menu_clean_dirty_mouse_internal, menu_is_mouse_over_option, &this->selectedOption)) {
            if (this->selectedOption == 0) {
                this->currentSubstate = MENU_FINISHED_PLAY;
            } else if (this->selectedOption == 1) {
                this->currentSubstate = MENU_MAIN; 
            } else if (this->selectedOption == 2) {
                this->currentSubstate = MENU_FINISHED_QUIT;
            }
        }
    }

     if (this->selectedOption != prevSelected) {
        base->draw(base);
    }
}

static void menu_destroy_internal(GameState *base) {
    Menu *this = (Menu *)base;
    if (this->uiRoot) {
        destroy_ui_component(this->uiRoot);
        this->uiRoot = NULL;
        titleText = NULL;
        playContainer = NULL;
        leaderboardContainer = NULL;
        quitContainer = NULL;
    }
    free(base);
}

Menu *menu_create() {
    Menu *this = (Menu *) malloc(sizeof(Menu));
    if (this == NULL) return NULL;
    
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))menu_draw_internal;
    this->base.process_event = menu_process;
    this->base.destroy = menu_destroy_internal;
    this->base.is_mouse_over = menu_is_mouse_over_option;

    this->selectedOption = 0;
    this->currentSubstate = MENU_MAIN;
    this->uiRoot = NULL;

    // Title 
    menuContainer = create_container_component(0, 0, vbe_mode_info.XResolution, vbe_mode_info.YResolution);
    if (!menuContainer) {
        free(this);
        return NULL;
    }
    set_container_layout(menuContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    Sprite *backgroundSprite = sprite_create_xpm((xpm_map_t) main_background, 0, 0, 0, 0);
    set_container_background_image(menuContainer, backgroundSprite);
    set_container_gap(menuContainer, 30);
    this->uiRoot = menuContainer;

    titleText = create_text_component("Byte Kart", gameFont, 0xFFFFFF);
    if (!titleText) {
        destroy_ui_component(menuContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(menuContainer, titleText);

    // Play option
    playContainer = create_container_component(0, 0, 200, 50);
    if (!playContainer) {
        destroy_ui_component(menuContainer);
        free(this);
        return NULL;
    }
    set_container_layout(playContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(playContainer, 0x111111);
    set_container_padding(playContainer, 20, 20, 20, 20);
    set_container_border_radius(playContainer, 10);
    set_container_border(playContainer, 2, 0xFFFFFF);
    playText = create_text_component("Play", gameFont, 0xFFFFFF);
    if (!playText) {
        destroy_ui_component(menuContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(playContainer, playText);
    add_child_to_container_component(menuContainer, playContainer);
    perform_container_layout(playContainer);

    // Leaderboard option
    leaderboardContainer = create_container_component(0, 0, 200, 50);
    if (!leaderboardContainer) {
        destroy_ui_component(menuContainer);
        free(this);
        return NULL;
    }
    set_container_layout(leaderboardContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(leaderboardContainer, 0x111111);
    set_container_padding(leaderboardContainer, 20, 20, 20, 20);
    set_container_border_radius(leaderboardContainer, 10);
    set_container_border(leaderboardContainer, 2, 0xFFFFFF);
    leaderboardText = create_text_component("Leaderboard", gameFont, 0xFFFFFF);
    if (!leaderboardText) {
        destroy_ui_component(menuContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(leaderboardContainer, leaderboardText);
    add_child_to_container_component(menuContainer, leaderboardContainer);
    perform_container_layout(leaderboardContainer);

    // Quit option
    quitContainer = create_container_component(0, 0, 200, 50);
    if (!quitContainer) {
        destroy_ui_component(menuContainer);
        free(this);
        return NULL;
    }
    set_container_layout(quitContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(quitContainer, 0x111111);
    set_container_padding(quitContainer, 20, 20, 20, 20);
    set_container_border_radius(quitContainer, 10);
    set_container_border(quitContainer, 2, 0xFFFFFF);
    quitText = create_text_component("Quit", gameFont, 0xFFFFFF);
    if (!quitText) {
        destroy_ui_component(menuContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(quitContainer, quitText);
    add_child_to_container_component(menuContainer, quitContainer);
    perform_container_layout(quitContainer);

    perform_container_layout(menuContainer);

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

