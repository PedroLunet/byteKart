#include <lcom/lcf.h>

#include "pause_menu.h"
#include "../game/cronometer.h"

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
static UIComponent *currentTime = NULL;
static UIComponent *quickControlsContainer = NULL;

static void pause_draw_internal(GameState *base) {
  Pause *this = (Pause *)base;
  if (this->uiRoot) {
    update_current_time_display();
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
    UIComponent *resume_option[] = {pauseOptions[0]};
    if (is_mouse_over_menu_options(base, mouse_x, mouse_y, resume_option, 1, selected_option, 0x28A745, 0x20C837)) {
        return true;
    }

    UIComponent *menu_option[] = {pauseOptions[1]};
    int temp_selection;
    if (is_mouse_over_menu_options(base, mouse_x, mouse_y, menu_option, 1, &temp_selection, 0xDC3545, 0xFF4757)) {
        *selected_option = 1;  
        return true;
    }

    set_container_background_color(pauseOptions[0], 0x28A745); 
    set_container_background_color(pauseOptions[1], 0xDC3545); 
    *selected_option = -1;
    return false;
}

static void pause_process(GameState *base, EventType event) {
    Pause *this = (Pause *)base;
    int prevSelected = this->selectedOption;
    if (event == EVENT_MOUSE) {
      bool hovered = false;
        if (base->handle_mouse_input(base, (void (*)(GameState *))pause_clean_dirty_mouse_internal, pause_is_mouse_over_option, &this->selectedOption)) {
          hovered = true;
            if (this->selectedOption == 0) {
              this->currentPauseSubstate = PAUSE_RESUME;
            } else if (this->selectedOption == 1) {
              this->currentPauseSubstate = PAUSE_MAIN_MENU;
            } 
        }
        if (this->selectedOption != prevSelected) {
            base->draw(base);
        }
    } else if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case P_KEY:
                this->currentPauseSubstate = PAUSE_RESUME;
                break;
            default:
                break;
        }
    }
}

static void pause_destroy_internal(GameState *base) {
    Pause *this = (Pause *)base;
    if (this->uiRoot) {
        destroy_ui_component(this->uiRoot);
        this->uiRoot = NULL;
        pauseText = NULL;
        quickControlsContainer = NULL;
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

    pauseContainer = create_main_container(NULL, 30, 0, 0, 0, 0);
    set_container_background_color(pauseContainer, 0x80000000);
    this->uiRoot = pauseContainer;

    // Title with better styling
    pauseText = create_title_text("GAME PAUSED", gameFont, 0xFFFFFF, pauseContainer);

    // Quick Controls container
    quickControlsContainer = create_container_component(0, 0, 0, 0);
    set_container_layout(quickControlsContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(quickControlsContainer, 0x1C1C1C);
    set_container_padding(quickControlsContainer, 15, 15, 15, 15);
    set_container_border(quickControlsContainer, 2, 0xFFDD00); 
    set_container_border_radius(quickControlsContainer, 8);
    set_container_gap(quickControlsContainer, 5); 

    // Game controls information
    create_title_text("Quick Controls", gameFont, 0xFFDD00, quickControlsContainer);
    create_title_text("P - Pause/Resume", gameFont, 0xAAAAAA, quickControlsContainer);
    create_title_text("Arrow Keys - Steer", gameFont, 0xAAAAAA, quickControlsContainer);

    add_child_to_container_component(pauseContainer, quickControlsContainer);

    // Display current time
    currentTime = display_current_time(pauseContainer);

    // Resume option
    resumeContainer = create_menu_option("Resume", gameFont, 200, 50, pauseContainer);
    pauseOptions[0] = resumeContainer;

    // Back to main menu option
    mainMenuContainer = create_menu_option("Back to Menu", gameFont, 200, 50, pauseContainer);
    pauseOptions[1] = mainMenuContainer;

    set_container_background_color(pauseOptions[0], 0x28A745); 
    set_container_background_color(pauseOptions[1], 0xDC3545); 

    perform_container_layout(pauseContainer);
    return this;
}

void pause_menu_destroy(Pause *this) {
  this->base.destroy(&this->base);
}

void pause_draw(Pause *this) {
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

