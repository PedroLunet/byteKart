#include <lcom/lcf.h>

#include "menu.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;
extern Font *gameFont;

static UIComponent *menuContainer = NULL;
static UIComponent *titleText = NULL;
static UIComponent *playContainer = NULL;
static UIComponent *leaderboardContainer = NULL;
static UIComponent *quitContainer = NULL;
static UIComponent *menuOptions[3]; 

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
    int *selected_option = (int *)data;
    return is_mouse_over_menu_options(base, mouse_x, mouse_y, menuOptions, 3, selected_option, 0x111111, 0xA81D1D);
}

static void menu_process(GameState *base, EventType event) {
    Menu *this = (Menu *)base;
    int prevSelected = this->selectedOption;
    if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))menu_clean_dirty_mouse_internal, menu_is_mouse_over_option, &this->selectedOption)) {
            if (this->selectedOption == 0) {
                this->currentSubstate = MENU_FINISHED_PLAY;
            } else if (this->selectedOption == 1) {
                this->currentSubstate = MENU_FINISHED_LEADERBOARD; 
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

    // Main Container
    Sprite *backgroundSprite = sprite_create_xpm((xpm_map_t) main_background, 0, 0, 0, 0);
    menuContainer = create_main_container(backgroundSprite, 30, 0, 0, 0, 0);
    this->uiRoot = menuContainer;

    // Title
    titleText = create_title_text("Byte Kart", gameFont, 0xFFFFFF, menuContainer);

    // Play option
    playContainer = create_menu_option("Play", gameFont, 200, 50, menuContainer);
    menuOptions[0] = playContainer;

    // Leaderboard option
    leaderboardContainer = create_menu_option("Leaderboard", gameFont, 200, 50, menuContainer);
    menuOptions[1] = leaderboardContainer;

    // Quit option
    quitContainer = create_menu_option("Quit", gameFont, 200, 50, menuContainer);
    menuOptions[2] = quitContainer;

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

