#include <lcom/lcf.h>

#include "leaderboard_menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;
extern Font *gameFont;

static UIComponent *leaderboardText = NULL;
static UIComponent *backButton = NULL;

static void leaderboard_draw_internal(GameState *base) {
    Leaderboard *this = (Leaderboard *)base;
    if (this->uiRoot) {
        draw_ui_component(this->uiRoot);
    }
    if (this->backButton) {
        draw_ui_component(this->backButton);
    }
}

static void leaderboard_clean_dirty_mouse_internal(GameState *base) {
    Leaderboard *this = (Leaderboard *)base;
    if (this->uiRoot) {
        draw_dirty_area(this->uiRoot, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
    if (this->backButton) {
        draw_dirty_area(this->backButton, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
}

static bool leaderboard_is_mouse_over(GameState *base, int mouse_x, int mouse_y, void *data) {
    Leaderboard *this = (Leaderboard *)base;
    int *selected = (int *)data;
    *selected = -1;
    if (this->backButton && this->backButton->type == TYPE_CONTAINER && this->backButton->data) {
        ContainerData *backButtonData = (ContainerData *)this->backButton->data;
        if (mouse_x >= this->backButton->x && mouse_x < this->backButton->x + backButtonData->width &&
            mouse_y >= this->backButton->y && mouse_y < this->backButton->y + backButtonData->height) {
                *selected = 0;
            return true;
        }
    }
    return false;
}

static void leaderboard_process(GameState *base, EventType event) {
    Leaderboard *this = (Leaderboard *)base;
    int selected = -1;
    if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))leaderboard_clean_dirty_mouse_internal, leaderboard_is_mouse_over, &selected)) {
            if (selected == 0) {
                this->currentSubstate = LEADERBOARD_BACK_TO_MENU;
            }
        }
    }
}

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

    this->currentSubstate = LEADERBOARD_MENU;

    // Initialize base GameState
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *)) leaderboard_draw_internal;
    this->base.process_event = leaderboard_process;
    this->base.destroy = leaderboard_destroy_internal;
    this->base.is_mouse_over = leaderboard_is_mouse_over;
    this->uiRoot = NULL;

    // Main Container
    UIComponent *leaderboardContainer = create_main_container(NULL, 30, 60, 60, 60, 60);
    this->uiRoot = leaderboardContainer;

    // Create the title text component
    leaderboardText = create_title_text("Leaderboard", gameFont, 0xFFFFFF, leaderboardContainer);

    // Create the back button
    backButton = create_back_button(gameFont, leaderboardContainer);
    
    // Create leaderboard box
    UIComponent *leaderboardBox = create_container_component(0, 0, 400, 400); 
    if (!leaderboardBox) {
        destroy_ui_component(leaderboardContainer);
        free(this);
        return NULL;
    }
    set_container_layout(leaderboardBox, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_START);
    set_container_background_color(leaderboardBox, 0x222222);
    set_container_padding(leaderboardBox, 0, 40, 40, 40);
    set_container_border_radius(leaderboardBox, 20);
    set_container_border(leaderboardBox, 4, 0xAA0000);
    set_container_hover_color(leaderboardBox, 0xAA0000);
    set_container_gap(leaderboardBox, 0);

    perform_container_layout(leaderboardBox);


    //header
    UIComponent *headerRow = create_container_component(0, 0, 280, 40);
    set_container_layout(headerRow, LAYOUT_ROW, ALIGN_CENTER, JUSTIFY_SPACE_BETWEEN);
    set_container_background_color(headerRow, 0x222222);
    set_container_gap(headerRow, 10);

    UIComponent *rankHeader = create_text_component("Rank", gameFont, 0xFFCC00);
    UIComponent *nameHeader = create_text_component("Name", gameFont, 0xFFCC00);
    UIComponent *scoreHeader = create_text_component("Score", gameFont, 0xFFCC00);

    add_child_to_container_component(headerRow, rankHeader);
    add_child_to_container_component(headerRow, nameHeader);
    add_child_to_container_component(headerRow, scoreHeader);
    
    add_child_to_container_component(leaderboardBox, headerRow);
    perform_container_layout(headerRow);



    // Exemplo
    const char *entries[] = {
        "1. Alice - 1500",
        "2. Bob - 1300",
        "3. Clara - 1100",
        "4. David - 900",
        "5. Eva - 750"
    };
    const int num_entries = sizeof(entries) / sizeof(entries[0]);

    for (int i = 0; i < num_entries; i++) {
        UIComponent *row = create_container_component(0, 0, 280, 40);
        set_container_layout(row, LAYOUT_ROW, ALIGN_START, JUSTIFY_SPACE_BETWEEN);
        set_container_background_color(row, 0x222222);
        set_container_gap(row, 10);
        
        int rank;
        char name[32];
        int score;
        sscanf(entries[i], "%d. %31s - %d", &rank, name, &score);
    
        char rank_str[4];
        sprintf(rank_str, "%d.", rank);
        UIComponent *rankText = create_text_component(rank_str, gameFont, 0xFFFFFF);
    
        UIComponent *nameText = create_text_component(name, gameFont, 0xFFFFFF);
    
        char score_str[12];
        sprintf(score_str, "%d", score);
        UIComponent *scoreText = create_text_component(score_str, gameFont, 0xFFFFFF);
    
        add_child_to_container_component(row, rankText);
        add_child_to_container_component(row, nameText);
        add_child_to_container_component(row, scoreText);
    
        add_child_to_container_component(leaderboardBox, row);
    }
    

    add_child_to_container_component(leaderboardContainer, leaderboardBox);
    perform_container_layout(leaderboardBox);
    
    perform_container_layout(backButton);

    perform_container_layout(leaderboardContainer);
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

