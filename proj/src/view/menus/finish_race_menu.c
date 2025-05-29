#include <lcom/lcf.h>

#include "finish_race_menu.h"
#include "view/game/game.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;
extern Font *gameFont;

static UIComponent *finishRaceContainer = NULL;
static UIComponent *finishRaceText = NULL;
static UIComponent *positionsContainer = NULL;
static UIComponent *mainMenuContainer = NULL;
static UIComponent *finishRaceOptions[1];

static void finish_race_draw_internal(GameState *base) {
  FinishRace *this = (FinishRace *)base;
  if (this->uiRoot) {
    draw_ui_component(this->uiRoot);
  }
}

static void finish_race_clean_dirty_mouse_internal(GameState *base) {
    FinishRace *this = (FinishRace *)base;
    if (this->uiRoot) {
        draw_dirty_area(this->uiRoot, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
}

static bool finish_race_is_mouse_over_option(GameState *base, int mouse_x, int mouse_y, void *data) {
    int *selected_option = (int *)data;
    UIComponent *menu_option[] = {finishRaceOptions[0]};
    if (is_mouse_over_menu_options(base, mouse_x, mouse_y, menu_option, 1, selected_option, 0xDC3545, 0xFF4757)) {
        return true;
    }

    set_container_background_color(finishRaceOptions[0], 0xDC3545); 
    *selected_option = -1;
    return false;
}

static void finish_race_process(GameState *base, EventType event) {
    FinishRace *this = (FinishRace *)base;
    int prevSelected = this->selectedOption;
    if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))finish_race_clean_dirty_mouse_internal, finish_race_is_mouse_over_option, &this->selectedOption)) {
            if (this->selectedOption == 0) {
              this->currentFinishRaceSubstate = FINISH_RACE_MAIN_MENU;
            }
        }
        if (this->selectedOption != prevSelected) {
            base->draw(base);
        }
    }
}

static void finish_race_destroy_internal(GameState *base) {
    FinishRace *this = (FinishRace *)base;
    if (this->uiRoot) {
        destroy_ui_component(this->uiRoot);
        this->uiRoot = NULL;
        finishRaceText = NULL;
    }
    free(base);
}

FinishRace *finish_race_menu_create(RaceResult *results, int total_results) {
    FinishRace *this = (FinishRace *) malloc(sizeof(FinishRace));
    if (this == NULL) return NULL;
    
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))finish_race_draw_internal;
    this->base.process_event = finish_race_process;
    this->base.destroy = finish_race_destroy_internal;
    this->base.is_mouse_over = finish_race_is_mouse_over_option;

    this->selectedOption = 0;
    this->currentFinishRaceSubstate = FINISH_RACE_MENU;
    this->uiRoot = NULL;

    finishRaceContainer = create_main_container(NULL, 30, 0, 0, 0, 0);
    set_container_background_color(finishRaceContainer, 0x80000000);
    this->uiRoot = finishRaceContainer;

    // Title
    finishRaceText = create_title_text("RACE FINISHED", gameFont, 0xFFFFFF, finishRaceContainer);

    if (results && total_results > 0) {
        char winner_message[50];
        sprintf(winner_message, "%s WINS!", results[0].name);
        UIComponent *winnerText = create_text_component(winner_message, gameFont, 0xFFDD00); 
        if (winnerText) {
            add_child_to_container_component(finishRaceContainer, winnerText);
        }
    }

    // Create positions container
    positionsContainer = create_container_component(0, 0, 450, 350);
    set_container_layout(positionsContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(positionsContainer, 0x1C1C1C);
    set_container_padding(positionsContainer, 20, 20, 20, 20);
    set_container_border(positionsContainer, 2, 0xFFDD00);
    set_container_border_radius(positionsContainer, 8);
    set_container_gap(positionsContainer, 8);

    create_title_text("Final Results", gameFont, 0xFFDD00, positionsContainer);
    
    add_child_to_container_component(finishRaceContainer, positionsContainer);

    if (results && total_results > 0) {
        for (int i = 0; i < total_results && i < 6; i++) {
            char position_text[100];
            uint32_t color = (i == 0) ? 0xFFD700 : // Gold for 1st place
                            (i == 1) ? 0xC0C0C0 : // Silver for 2nd place
                            (i == 2) ? 0xCD7F32 : // Bronze for 3rd place
                            0xFFFFFF;              // White for others
            
            if (strcmp(results[i].name, "Player") == 0) {
                sprintf(position_text, "%d. %s (%.2fs) ★", 
                       results[i].position, results[i].name, results[i].race_time);
            } else {
                sprintf(position_text, "%d. %s (%.2fs)", 
                       results[i].position, results[i].name, results[i].race_time);
            }
            
            UIComponent *positionText = create_text_component(position_text, gameFont, color);
            if (positionText) {
                add_child_to_container_component(positionsContainer, positionText);
            }
        }
    }

    // Back to main menu option
    mainMenuContainer = create_menu_option("Back to Menu", gameFont, 200, 50, finishRaceContainer);
    finishRaceOptions[0] = mainMenuContainer;
    set_container_background_color(finishRaceOptions[0], 0xDC3545);

    perform_container_layout(finishRaceContainer);
    return this;
}

void finish_race_menu_destroy(FinishRace *this) {
  this->base.destroy(&this->base);
}

void finish_race_draw(FinishRace *this) {
  this->base.draw(&this->base);
  this->base.draw_mouse(&this->base);
}

void finish_race_process_event(FinishRace *this, EventType event) {
  this->base.process_event(&this->base, event);
}

FinishRaceSubstate finish_race_get_current_substate(FinishRace *this) {
  return this->currentFinishRaceSubstate;
}

void finish_race_reset_state(FinishRace *this) {
  this->currentFinishRaceSubstate = FINISH_RACE_MENU;
  this->selectedOption = 0;
  this->base.draw(&this->base);
}
