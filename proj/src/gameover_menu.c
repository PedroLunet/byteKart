#include <lcom/lcf.h>

#include "gameover_menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;
extern Font *gameFont;

static UIComponent *mainContainer = NULL;
static UIComponent *gameoverText = NULL;
static UIComponent *restartText =  NULL;
static UIComponent *restartContainer = NULL;
static UIComponent *mainMenuText = NULL;
static UIComponent *mainMenuContainer = NULL;
static UIComponent *quitText = NULL;
static UIComponent *quitContainer = NULL;

static void gameover_draw_internal(GameState *base) {
  GameOver *this = (GameOver *)base;
  if (this->uiRoot) {
    draw_ui_component(this->uiRoot);
  }
}


static bool gameover_is_mouse_over_option(GameState *base, int mouse_x, int mouse_y, void *data) {
    GameOver *this = (GameOver *)base;
    int *selected_option = (int *)data;
    *selected_option = -1;

    if (this->uiRoot && this->uiRoot->type == TYPE_CONTAINER) {
        ContainerData *rootData = (ContainerData *)
        this->uiRoot->data;

        if (rootData->num_children >= 4) {
            UIComponent *restart = rootData->children[1];
            UIComponent *mainMenu = rootData->children[2];
            UIComponent *quit = rootData->children[3];

            if (restart && restart->type == TYPE_CONTAINER && restart->data) {
                ContainerData *restartData = (ContainerData *)restart->data;
                if (mouse_x >= restart->x && mouse_x < restart->x + restartData->width &&
                    mouse_y >= restart->y && mouse_y < restart->y + restartData->height) {
                    *selected_option = 0;
                    set_container_background_color(restartContainer, 0xA81D1D);
                    return true;
                }
            }
            if (mainMenu && mainMenu->type == TYPE_CONTAINER && mainMenu->data) {
                ContainerData *mainMenuData = (ContainerData *)mainMenu->data;
                if (mouse_x >= mainMenu->x && mouse_x < mainMenu->x + mainMenuData->width &&
                    mouse_y >= mainMenu->y && mouse_y < mainMenu->y + mainMenuData->height) {
                    *selected_option = 1;
                    set_container_background_color(mainMenuContainer, 0xA81D1D);
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
                set_container_background_color(restartContainer, 0x111111);
            }
            if (*selected_option != 1) {
                set_container_background_color(mainMenuContainer, 0x111111);
            }
            if (*selected_option != 2) {
                set_container_background_color(quitContainer, 0x111111);
            }
        }
    }

    return false;
}

static void gameover_process(GameState *base, EventType event) {
    GameOver *this = (GameOver *)base;
    if (event == EVENT_MOUSE) {
        int prevSelected = this->selectedOption;
        if (base->handle_mouse_input(base, (void (*)(GameState *))gameover_draw, gameover_is_mouse_over_option, &this->selectedOption)) {
            if (this->selectedOption == 0) {
                this->currentGameOverSubstate = GAMEOVER_RESTART;
            } else if (this->selectedOption == 1) {
                this->currentGameOverSubstate = GAMEOVER_MAIN_MENU; 
            } else if (this->selectedOption == 2) {
                this->currentGameOverSubstate = GAMEOVER_QUIT_BUTTON;
            }
        }
        if (this->selectedOption != prevSelected) {
            base->draw(base);
        }
    }
}

static void gameover_destroy_internal(GameState *base) {
    GameOver *this = (GameOver *)base;
    if (this->uiRoot) {
        destroy_ui_component(this->uiRoot);
        this->uiRoot = NULL;
        gameoverText = NULL;
        restartContainer = NULL;
        mainMenuContainer = NULL;
        quitContainer = NULL;
    }
    free(base);
}


GameOver *gameover_menu_create() {
    GameOver *this = (GameOver *) malloc(sizeof(GameOver));
    if (this == NULL) return NULL;
    
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))gameover_draw_internal;
    this->base.process_event = gameover_process;
    this->base.destroy = gameover_destroy_internal;
    this->base.is_mouse_over = gameover_is_mouse_over_option;

    this->selectedOption = 0;
    this->currentGameOverSubstate = GAMEOVER_MENU;
    this->uiRoot = NULL;

    // Game Over Text
    mainContainer = create_container_component(0, 0, vbe_mode_info.XResolution, vbe_mode_info.YResolution);
    if (!mainContainer) {
        free(this);
        return NULL;
    }
    set_container_layout(mainContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(mainContainer, 0x111111);
    set_container_gap(mainContainer, 30);
    this->uiRoot = mainContainer;

    gameoverText = create_text_component("Game Over!", gameFont, 0xFFFFFF);
    if (!gameoverText) {
        destroy_ui_component(mainContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(mainContainer, gameoverText);

    // Restart option
    restartContainer = create_container_component(0, 0, 200, 50);
    if (!restartContainer) {
        destroy_ui_component(mainContainer);
        free(this);
        return NULL;
    }
    set_container_layout(restartContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(restartContainer, 0x111111);
    set_container_padding(restartContainer, 20, 20, 20, 20);
    set_container_border_radius(restartContainer, 10);
    set_container_border(restartContainer, 2, 0xFFFFFF);
    restartText = create_text_component("Restart", gameFont, 0xFFFFFF);
    if (!restartText) {
        destroy_ui_component(mainContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(restartContainer, restartText);
    add_child_to_container_component(mainContainer, restartContainer);
    perform_container_layout(restartContainer);

    // Back to main menu option
    mainMenuContainer = create_container_component(0, 0, 200, 50);
    if (!mainMenuContainer) {
        destroy_ui_component(mainContainer);
        free(this);
        return NULL;
    }
    set_container_layout(mainMenuContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(mainMenuContainer, 0x111111);
    set_container_padding(mainMenuContainer, 20, 20, 20, 20);
    set_container_border_radius(mainMenuContainer, 10);
    set_container_border(mainMenuContainer, 2, 0xFFFFFF);
    mainMenuText = create_text_component("Back to Menu", gameFont, 0xFFFFFF);
    if (!mainMenuText) {
        destroy_ui_component(mainContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(mainMenuContainer, mainMenuText);
    add_child_to_container_component(mainContainer, mainMenuContainer);
    perform_container_layout(mainMenuContainer);

    // Quit option
    quitContainer = create_container_component(0, 0, 200, 50);
    if (!quitContainer) {
        destroy_ui_component(mainContainer);
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
        destroy_ui_component(mainContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(quitContainer, quitText);
    add_child_to_container_component(mainContainer, quitContainer);
    perform_container_layout(quitContainer);

    perform_container_layout(mainContainer);

    return this;
}


void gameover_menu_destroy(GameOver *this) {
  this->base.destroy(&this->base);
}


void gameover_draw(GameOver *this) {
  this->base.clear_mouse_area(&this->base);
  this->base.draw(&this->base);
  this->base.draw_mouse(&this->base);
}


void gameover_process_event(GameOver *this, EventType event) {
  this->base.process_event(&this->base, event);
}


GameOverSubstate gameover_get_current_substate(GameOver *this) {
  return this->currentGameOverSubstate;
}


void gameover_reset_state(GameOver *this) {
  this->currentGameOverSubstate = GAMEOVER_MENU;
  this->selectedOption = 0;
  this->base.draw(&this->base);
}

