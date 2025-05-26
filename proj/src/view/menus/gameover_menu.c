#include <lcom/lcf.h>

#include "gameover_menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;
extern Font *gameFont;

static UIComponent *mainContainer = NULL;
static UIComponent *gameoverText = NULL;
static UIComponent *restartContainer = NULL;
static UIComponent *mainMenuContainer = NULL;
static UIComponent *quitContainer = NULL;
static UIComponent *gameoverOptions[3];

static void gameover_draw_internal(GameState *base) {
  GameOver *this = (GameOver *)base;
  if (this->uiRoot) {
    draw_ui_component(this->uiRoot);
  }
}


static void gameover_clean_dirty_mouse_internal(GameState *base) {
    GameOver *this = (GameOver *)base;
    if (this->uiRoot) {
        draw_dirty_area(this->uiRoot, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
}


static bool gameover_is_mouse_over_option(GameState *base, int mouse_x, int mouse_y, void *data) {
    int *selected_option = (int *)data;
    return is_mouse_over_menu_options(base, mouse_x, mouse_y, gameoverOptions, 3, selected_option, 0x111111, 0xA81D1D);
}

static void gameover_process(GameState *base, EventType event) {
    GameOver *this = (GameOver *)base;
    int prevSelected = this->selectedOption;
    if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))gameover_clean_dirty_mouse_internal, gameover_is_mouse_over_option, &this->selectedOption)) {
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
    mainContainer = create_main_container(NULL, 30, 0, 0, 0, 0);
    this->uiRoot = mainContainer;

    // Title 
    gameoverText = create_title_text("Game Over!", gameFont, 0xFFFFFF, mainContainer);

    // Restart option
    restartContainer = create_menu_option("Restart", gameFont, 200, 50, mainContainer);
    gameoverOptions[0] = restartContainer;

    // Back to main menu option
    mainMenuContainer = create_menu_option("Back to Menu", gameFont, 200, 50, mainContainer);
    gameoverOptions[1] = mainMenuContainer;

    // Quit option
    quitContainer = create_menu_option("Quit", gameFont, 200, 50, mainContainer);
    gameoverOptions[2] = quitContainer;

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

