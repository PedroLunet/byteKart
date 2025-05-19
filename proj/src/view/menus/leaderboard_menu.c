#include <lcom/lcf.h>

#include "leaderboard_menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;
extern Font *gameFont;

static UIComponent *leaderboardText = NULL;
static UIComponent *backButton = NULL;
static UIComponent *backText = NULL;


static void leaderboard_draw_internal(GameState *base) {
    Leaderboard *this = (Leaderboard *)base;
    if (this->uiRoot) {
        draw_ui_component(this->uiRoot);
    }
    if (this->backButton) {
        draw_ui_component(this->backButton);
    }
}

/*
static void leaderboard_clean_dirty_mouse_internal(GameState *base) {
    Leaderboard *this = (Leaderboard *)base;
    if (this->uiRoot) {
        draw_dirty_area(this->uiRoot, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
    if (this->backButton) {
        draw_dirty_area(this->backButton, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
}
*/

// TODO: is_mouse_hover
// TODO: leaderboard_process

static void leaderboard_destroy_internal(GameState *base) {
    Leaderboard *this = (Leaderboard *)base;
    if (this->uiRoot) {
        destroy_ui_component(this->uiRoot);
        this->uiRoot = NULL;
        leaderboardText = NULL;
    }
    free(base);
}

Leaderboard *leaderboard_create() {
    Leaderboard *this = (Leaderboard *)malloc(sizeof(Leaderboard));
    if (!this) return NULL;

    // Initialize base GameState
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *)) leaderboard_draw_internal;
    //this->base.process_event = 
    this->base.destroy = leaderboard_destroy_internal;
    //this->base.is_mouse_over = leaderboard_is_mouse_over;
    this->uiRoot = NULL;

    // Create UI Components
    UIComponent *leaderboardContainer = create_container_component(0, 0, vbe_mode_info.XResolution, vbe_mode_info.YResolution);
    if (!leaderboardContainer) {
        free(this);
        return NULL;
    }
    set_container_layout(leaderboardContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(leaderboardContainer, 0x111111);
    set_container_padding(leaderboardContainer, 60, 60, 60, 60);
    set_container_gap(leaderboardContainer, 30);
    this->uiRoot = leaderboardContainer;

    // Create the title text component
    leaderboardText = create_text_component("Leaderboard", gameFont, 0xFFFFFF);
    if (!leaderboardText) {
        destroy_ui_component(leaderboardContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(leaderboardContainer, leaderboardText);

    // Create the back button
    backButton = create_container_component(30, 30, 40, 40);
    if (!backButton) {
        destroy_ui_component(leaderboardContainer);
        free(this);
        return NULL;
    }
    set_container_layout(backButton, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(backButton, 0x00BB00);
    set_container_hover_color(backButton, 0x00DD00);
    set_container_border_radius(backButton, 15);
    set_container_border(backButton, 2, 0x00DD00);
    backText = create_text_component("<", gameFont, 0xFFFFFF);
    if (!backText) {
        destroy_ui_component(leaderboardContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(backButton, backText);

    perform_container_layout(backButton);
    this->backButton = backButton;

    return this;
}

void leaderboard_destroy(Leaderboard *this) {
  this->base.destroy(&this->base);
}

void leaderboard_draw(Leaderboard *this) {
  this->base.draw(&this->base);
}

void leaderboard_process_event(Leaderboard *this, EventType event) {
  this->base.process_event(&this->base, event);
}

LeaderboardSubstate leaderboard_get_current_substate(Leaderboard *this) {
    return this->currentSubstate;
}

void leaderboard_reset_state(Leaderboard *this) {
  this->currentSubstate = LEADERBOARD_MENU;
  this->base.draw(&this->base);
}

