#include <lcom/lcf.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "select_difficulty.h"
#include "controller/video_card.h"
#include "controller/mouse.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern struct packet pp;
extern Font *gameFont;

// UI Components
static UIComponent *titleText = NULL;
static UIComponent *optionsRowContainer = NULL;
static UIComponent *easyContainer = NULL;
static UIComponent *mediumContainer = NULL;
static UIComponent *hardContainer = NULL;
static UIComponent *easyOption = NULL;
static UIComponent *mediumOption = NULL;
static UIComponent *hardOption = NULL;

static void select_difficulty_draw_internal(GameState *base) {
    SelectDifficulty *this = (SelectDifficulty *)base;
    if (this->uiRoot) {
        draw_ui_component(this->uiRoot);
    }
}

static bool select_difficulty_is_mouse_over(GameState *base, int mouse_x, int mouse_y, void *data) {
    SelectDifficulty *this = (SelectDifficulty *)base;
    int *selected = (int *)data;
    *selected = -1;

    if (this->uiRoot && this->uiRoot->type == TYPE_CONTAINER && this->uiRoot->data) {
        ContainerData *difficultyContainerData = (ContainerData *)this->uiRoot->data;

        if (difficultyContainerData->num_children > 1 && difficultyContainerData->children[1] && difficultyContainerData->children[1]->type == TYPE_CONTAINER && difficultyContainerData->children[1]->data) {
            ContainerData *optionsRowContainerData = (ContainerData *)difficultyContainerData->children[1]->data;

            if (optionsRowContainerData->num_children >= 3) {
                UIComponent *easy = ((ContainerData *)optionsRowContainerData->children[0]->data)->children[0];
				UIComponent *medium = ((ContainerData *)optionsRowContainerData->children[1]->data)->children[0];
				UIComponent *hard = ((ContainerData *)optionsRowContainerData->children[2]->data)->children[0];

                if (easy && easy->type == TYPE_TEXT && easy->data) {
                    TextElementData *easyData = (TextElementData *)easy->data;
                    if (mouse_x >= easy->x && mouse_x < easy->x + easyData->width &&
                        mouse_y >= easy->y && mouse_y < easy->y + easyData->height) {
                        *selected = 0;
                        return true;
                    }
                }

                if (medium && medium->type == TYPE_TEXT && medium->data) {
                    TextElementData *mediumData = (TextElementData *)medium->data;
                    if (mouse_x >= medium->x && mouse_x < medium->x + mediumData->width &&
                        mouse_y >= medium->y && mouse_y < medium->y + mediumData->height) {
                        *selected = 1;
                        return true;
                    }
                }

                if (hard && hard->type == TYPE_TEXT && hard->data) {
                    TextElementData *hardData = (TextElementData *)hard->data;
                    if (mouse_x >= hard->x && mouse_x < hard->x + hardData->width &&
                        mouse_y >= hard->y && mouse_y < hard->y + hardData->height) {
                        *selected = 2;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

static void select_difficulty_process(GameState *base, EventType event) {
    SelectDifficulty *this = (SelectDifficulty *)base;
    if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case UP_ARROW:
                if (this->selectedOption > 0) this->selectedOption--;
            break;
            case DOWN_ARROW:
                if (this->selectedOption < 2) this->selectedOption++;
            break;
            case ENTER_KEY:
                if (this->selectedOption == 0) this->chosenLevel = DIFFICULTY_EASY;
                else if (this->selectedOption == 1) this->chosenLevel = DIFFICULTY_MEDIUM;
                else if (this->selectedOption == 2) this->chosenLevel = DIFFICULTY_HARD;
            break;
            case ESC_BREAKCODE:
                this->chosenLevel = DIFFICULTY_EXITED;
            break;
            default:
                break;
        }
        base->draw(base);
    } else if (event == EVENT_MOUSE) {
        int prevSelected = this->selectedOption;
        if (base->handle_mouse_input(base, (void (*)(GameState *))select_difficulty_draw_internal, select_difficulty_is_mouse_over, &this->selectedOption)) {
            if (this->selectedOption != -1) {
                if (this->selectedOption == 0) this->chosenLevel = DIFFICULTY_EASY;
                else if (this->selectedOption == 1) this->chosenLevel = DIFFICULTY_MEDIUM;
                else if (this->selectedOption == 2) this->chosenLevel = DIFFICULTY_HARD;
            }
        }
        if (this->selectedOption != prevSelected) {
            base->draw(base);
        }
    }
}

static void select_difficulty_destroy_internal(GameState *base) {
    SelectDifficulty *this = (SelectDifficulty *)base;
    if (this->uiRoot) {
        destroy_ui_component(this->uiRoot);
        this->uiRoot = NULL;
        titleText = NULL;
        easyOption = NULL;
        mediumOption = NULL;
        hardOption = NULL;
    }
    free(base);
}

SelectDifficulty *select_difficulty_create() {
    SelectDifficulty *this = (SelectDifficulty *)malloc(sizeof(SelectDifficulty));
    if (!this) return NULL;

    // Initialize base GameState
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))select_difficulty_draw_internal;
    this->base.process_event = select_difficulty_process;
    this->base.destroy = select_difficulty_destroy_internal;
    this->base.is_mouse_over = select_difficulty_is_mouse_over;

    this->selectedOption = 0;
    this->chosenLevel = DIFFICULTY_START;
    this->uiRoot = NULL;

    // Create UI Components
    UIComponent *difficultyContainer = create_container_component(0, 0, vbe_mode_info.XResolution, vbe_mode_info.YResolution);
    if (!difficultyContainer) {
        free(this);
        return NULL;
    }
    set_container_layout(difficultyContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(difficultyContainer, 0x111111);
    set_container_padding(difficultyContainer, 60, 60, 60, 60);
    set_container_gap(difficultyContainer, 30);
    this->uiRoot = difficultyContainer;


    // Create the title text component
    titleText = create_text_component("Select Difficulty", gameFont, 0xFFFFFF);
    if (!titleText) {
        destroy_ui_component(difficultyContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(difficultyContainer, titleText);


    // Create a container for the difficulty options in a row
    optionsRowContainer = create_container_component(0, 0, 0, 0);
    if (!optionsRowContainer) {
        destroy_ui_component(difficultyContainer);
        free(this);
        return NULL;
    }
    set_container_layout(optionsRowContainer, LAYOUT_ROW, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_gap(optionsRowContainer, 30);
    set_container_background_color(optionsRowContainer, 0x111111);
    add_child_to_container_component(difficultyContainer, optionsRowContainer);

    // Create the difficulty option components
    easyContainer = create_container_component(0, 0, 200, 300);
    if (!easyContainer) {
        destroy_ui_component(difficultyContainer);
        free(this);
        return NULL;
    }
    set_container_layout(easyContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_END);
    set_container_background_color(easyContainer, 0x111111);
    set_container_padding(easyContainer, 40, 40, 40, 40);
    set_container_border_radius(easyContainer, 20);
    set_container_border(easyContainer, 4, 0xAA0000);
    easyOption = create_text_component("Easy", gameFont, 0xFFFFFF);

    if (!easyOption) {
        destroy_ui_component(difficultyContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(easyContainer, easyOption);
    add_child_to_container_component(optionsRowContainer, easyContainer);
    perform_container_layout(easyContainer);

    mediumContainer = create_container_component(0, 0, 200, 300);
    if (!mediumContainer) {
        destroy_ui_component(difficultyContainer);
        free(this);
        return NULL;
    }
    set_container_layout(mediumContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_END);
    set_container_background_color(mediumContainer, 0x111111);
    set_container_padding(mediumContainer, 40, 40, 40, 40);
    set_container_border_radius(mediumContainer, 20);
    set_container_border(mediumContainer, 4, 0xAA0000);
    mediumOption = create_text_component("Medium", gameFont, 0xFFFFFF);

    if (!mediumOption) {
        destroy_ui_component(difficultyContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(mediumContainer, mediumOption);
    add_child_to_container_component(optionsRowContainer, mediumContainer);
    perform_container_layout(mediumContainer);

    hardContainer = create_container_component(0, 0, 200, 300);
    if (!hardContainer) {
        destroy_ui_component(difficultyContainer);
        free(this);
        return NULL;
    }
    set_container_layout(hardContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_END);
    set_container_background_color(hardContainer, 0x111111);
    set_container_padding(hardContainer, 40, 40, 40, 40);
    set_container_border_radius(hardContainer, 20);
    set_container_border(hardContainer, 4, 0xAA0000);
    hardOption = create_text_component("Hard", gameFont, 0xFFFFFF);
    if (!hardOption) {
        destroy_ui_component(difficultyContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(hardContainer, hardOption);
    add_child_to_container_component(optionsRowContainer, hardContainer);
    perform_container_layout(hardContainer);

    perform_container_layout(optionsRowContainer);
    perform_container_layout(difficultyContainer);

    return this;
}

void select_difficulty_destroy(SelectDifficulty *this) {
    this->base.destroy(&this->base);
}

void select_difficulty_draw(SelectDifficulty *this) {
    this->base.draw(&this->base);
}

void select_difficulty_process_event(SelectDifficulty *this, EventType event) {
    this->base.process_event(&this->base, event);
}

DifficultyLevel select_difficulty_get_chosen_level(SelectDifficulty *this) {
    return this->chosenLevel;
}

void select_difficulty_reset_state(SelectDifficulty *this) {
    this->selectedOption = 0;
    this->chosenLevel = DIFFICULTY_START;
    this->base.draw(&this->base);
}

