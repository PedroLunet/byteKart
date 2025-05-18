#include <lcom/lcf.h>

#include "pause_menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;
extern Font *gameFont;

static UIComponent *pauseContainer = NULL;
static UIComponent *pauseText = NULL;
static UIComponent *resumeContainer = NULL;
static UIComponent *mainMenuContainer = NULL;
static UIComponent *pauseOptions[2];

static void pause_draw_internal(GameState *base) {
  Pause *this = (Pause *)base;
  if (this->uiRoot) {
    draw_ui_component(this->uiRoot);
  }
}

static void pause_clean_dirty_mouse_internal(GameState *base) {
    Pause *this = (Pause *)base;
    if (this->uiRoot) {
        draw_dirty_area(this->uiRoot, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
}

static bool pause_is_mouse_over_option(GameState *base, int mouse_x, int mouse_y, void *data) {
    int *selected_option = (int *)data;
    return is_mouse_over_menu_options(base, mouse_x, mouse_y, pauseOptions, 2, selected_option, 0x111111, 0xA81D1D);
}

static void pause_process(GameState *base, EventType event) {
    Pause *this = (Pause *)base;
    int prevSelected = this->selectedOption;
    if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))pause_clean_dirty_mouse_internal, pause_is_mouse_over_option, &this->selectedOption)) {
            if (this->selectedOption == 0) {
              this->currentPauseSubstate = PAUSE_RESUME;
            } else if (this->selectedOption == 1) {
              this->currentPauseSubstate = PAUSE_MAIN_MENU;
            } 
        }
        if (this->selectedOption != prevSelected) {
            base->draw(base);
        }
    }
}

static void pause_destroy_internal(GameState *base) {
    Pause *this = (Pause *)base;
    if (this->uiRoot) {
        destroy_ui_component(this->uiRoot);
        this->uiRoot = NULL;
        pauseText = NULL;
    }
    free(base);
}

Pause *pause_menu_create() {
    Pause *this = (Pause *) malloc(sizeof(Pause));
    if (this == NULL) return NULL;
    
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))pause_draw_internal;
    this->base.process_event = pause_process;
    this->base.destroy = pause_destroy_internal;
    this->base.is_mouse_over = pause_is_mouse_over_option;

    this->selectedOption = 0;
    this->currentPauseSubstate = PAUSE_MENU;
    this->uiRoot = NULL;

    // Pause Text
    pauseContainer = create_container_component(0, 0, vbe_mode_info.XResolution, vbe_mode_info.YResolution);
    if (!pauseContainer) {
        free(this);
        return NULL;
    }
    set_container_layout(pauseContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(pauseContainer, 0x111111);
    set_container_gap(pauseContainer, 30);
    this->uiRoot = pauseContainer;

    pauseText = create_text_component("Game Paused", gameFont, 0xFFFFFF);
    if (!pauseText) {
        destroy_ui_component(pauseContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(pauseContainer, pauseText);

    // Resume option
    resumeContainer = create_menu_option("Resume", gameFont, 200, 50, pauseContainer);
        if (!resumeContainer) {
            destroy_ui_component(pauseContainer);
            free(this);
            return NULL;
        }
    pauseOptions[0] = resumeContainer;

    // Back to main menu option
    mainMenuContainer = create_menu_option("Back to Menu", gameFont, 200, 50, pauseContainer);
    if (!mainMenuContainer) {
        destroy_ui_component(pauseContainer);
        free(this);
        return NULL;
    }
    pauseOptions[1] = mainMenuContainer;

    perform_container_layout(pauseContainer);
    return this;
}

void pause_menu_destroy(Pause *this) {
  this->base.destroy(&this->base);
}

void pause_draw(Pause *this) {
  this->base.clear_mouse_area(&this->base);
  this->base.draw(&this->base);
  this->base.draw_mouse(&this->base);
}

void pause_process_event(Pause *this, EventType event) {
  this->base.process_event(&this->base, event);
}

PauseSubstate pause_get_current_substate(Pause *this) {
  return this->currentPauseSubstate;
}

void pause_reset_state(Pause *this) {
  this->currentPauseSubstate = PAUSE_MENU;
  this->selectedOption = 0;
  this->base.draw(&this->base);
}

