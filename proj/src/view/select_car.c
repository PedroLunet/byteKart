#include <lcom/lcf.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "select_car.h"
#include "controller/video_card.h"
#include "controller/mouse.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern struct packet pp;
extern Font *gameFont;

// UI Components
static UIComponent *titleText = NULL;
static UIComponent *optionsRowContainer = NULL;
static UIComponent *firstRowOptions = NULL;
static UIComponent *secondRowOptions = NULL;
static UIComponent *backButton = NULL;
static UIComponent *backText = NULL;

static UIComponent *containers[10];

static void select_car_draw_internal(GameState *base) {
    SelectCar *this = (SelectCar *)base;
    if (this->uiRoot) {
        draw_ui_component(this->uiRoot);
    }
    if (this->backButton) {
        draw_ui_component(this->backButton);
    }
}

static void select_car_clean_dirty_mouse_internal(GameState *base) {
    SelectCar *this = (SelectCar *)base;
    if (this->uiRoot) {
        draw_dirty_area(this->uiRoot, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
    if (this->backButton) {
        draw_dirty_area(this->backButton, base->prev_mouse_x, base->prev_mouse_y, base->prev_cursor_width, base->prev_cursor_height);
    }
}

static bool select_car_is_mouse_over(GameState *base, int mouse_x, int mouse_y, void *data) {
    SelectCar *this = (SelectCar *)base;
    int *selected = (int *)data;
    *selected = -1;

    if (this->uiRoot && this->uiRoot->type == TYPE_CONTAINER && this->uiRoot->data) {
          ContainerData *carContainerData = (ContainerData *)this->uiRoot->data;

          if (carContainerData->num_children > 1 && carContainerData->children[1] && carContainerData->children[1]->type == TYPE_CONTAINER && carContainerData->children[1]->data) {
              ContainerData *optionsRowContainerData = (ContainerData *)carContainerData->children[1]->data;

              for (int row = 0; row < 2; row++) {
                  if (optionsRowContainerData->num_children > row) {
                      UIComponent *rowContainer = optionsRowContainerData->children[row];
                      if (rowContainer && rowContainer->type == TYPE_CONTAINER && rowContainer->data) {
                          ContainerData *rowData = (ContainerData *)rowContainer->data;

                          for (int col = 0; col < rowData->num_children; col++) {
                              UIComponent *indicidualCarContainer = rowData->children[col];
                              if (indicidualCarContainer && indicidualCarContainer->type == TYPE_CONTAINER && indicidualCarContainer->data) {
                                 ContainerData *carData = (ContainerData *)indicidualCarContainer->data;
                                 if (mouse_x >= indicidualCarContainer->x && mouse_x < indicidualCarContainer->x + carData->width &&
                                     mouse_y >= indicidualCarContainer->y && mouse_y < indicidualCarContainer->y + carData->height) {
                                     *selected = row * 5 + col;
                                     return true;
                                 }
                              }
                          }
                      }
                  }
              }
          }

    }

    if (this->backButton && this->backButton->type == TYPE_CONTAINER && this->backButton->data) {
        ContainerData *backButtonData = (ContainerData *)this->backButton->data;
        if (mouse_x >= this->backButton->x && mouse_x < this->backButton->x + backButtonData->width &&
            mouse_y >= this->backButton->y && mouse_y < this->backButton->y + backButtonData->height) {
            *selected = 10;
            return true;
        }
    }

    return false;
}

static void select_car_process(GameState *base, EventType event) {
    SelectCar *this = (SelectCar *)base;
    int prevSelected = this->selectedOption;
    if (event == EVENT_KEYBOARD) {
        int matrix_x = this->selectedOption % 5;
        int matrix_y = this->selectedOption / 5;

        switch (scancode) {
            case LEFT_ARROW:
                if (matrix_x > 0) this->selectedOption--;
            break;
            case RIGHT_ARROW:
                if (matrix_x < 4) this->selectedOption++;
            break;
            case UP_ARROW:
                if (matrix_y < 1) this->selectedOption = 10;
                if (matrix_y == 1) this->selectedOption -= 5;
            break;
            case DOWN_ARROW:
                if (matrix_y == 0) this->selectedOption += 5;
                if (matrix_y == 2) this->selectedOption = 0;
            break;
            case ENTER_KEY:
                this->chosenLevel = CAR_SELECTED;
            break;
            case ESC_BREAKCODE:
                this->chosenLevel = CAR_EXITED;
            break;
            default:
                break;
        }
    } else if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))select_car_clean_dirty_mouse_internal, select_car_is_mouse_over, &this->selectedOption)) {
            this->chosenLevel = CAR_SELECTED;
        }
    }

    if (this->selectedOption >= 0 && this->selectedOption < 10 && containers[this->selectedOption] != NULL) {
        is_container_hovered(containers[this->selectedOption]);
    } else if (this->selectedOption == 10 && backButton != NULL) {
        is_container_hovered(backButton);
    }

    if (this->selectedOption != prevSelected) {
        base->draw(base);
    }
}

static void select_car_destroy_internal(GameState *base) {
    SelectCar *this = (SelectCar *)base;
    if (this->uiRoot) {
        destroy_ui_component(this->uiRoot);
        this->uiRoot = NULL;
        titleText = NULL;
        optionsRowContainer = NULL;
        firstRowOptions = NULL;
        secondRowOptions = NULL;
    }
    free(base);
}

SelectCar *select_car_create() {
    SelectCar *this = (SelectCar *)malloc(sizeof(SelectCar));
    if (!this) return NULL;

    // Initialize base GameState
    init_base_game_state(&this->base);
    this->base.draw = (void (*)(GameState *))select_car_draw_internal;
    this->base.process_event = select_car_process;
    this->base.destroy = select_car_destroy_internal;
    this->base.is_mouse_over = select_car_is_mouse_over;

    this->selectedOption = -1;
    this->chosenLevel = CAR_START;
    this->uiRoot = NULL;
    this->backButton = NULL;

    // Create UI Components
    UIComponent *carContainer = create_container_component(0, 0, vbe_mode_info.XResolution, vbe_mode_info.YResolution);
    if (!carContainer) {
        free(this);
        return NULL;
    }
    set_container_layout(carContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(carContainer, 0x111111);
    set_container_padding(carContainer, 0, 0, 60, 60);
    set_container_gap(carContainer, 30);
    this->uiRoot = carContainer;


    // Create the title text component
    titleText = create_text_component("Select your car", gameFont, 0xFFFFFF);
    if (!titleText) {
        destroy_ui_component(carContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(carContainer, titleText);


    // Create a container for the car options in a row
    optionsRowContainer = create_container_component(0, 0, 0, 0);
    if (!optionsRowContainer) {
        destroy_ui_component(carContainer);
        free(this);
        return NULL;
    }
    set_container_layout(optionsRowContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_gap(optionsRowContainer, 20);
    set_container_background_color(optionsRowContainer, 0x111111);
    add_child_to_container_component(carContainer, optionsRowContainer);

        // Create the first row of car options
        firstRowOptions = create_container_component(0, 0, 0, 0);
        secondRowOptions = create_container_component(0, 0, 0, 0);
        if (!firstRowOptions || !secondRowOptions) {
            destroy_ui_component(carContainer);
            free(this);
            return NULL;
        }
        set_container_layout(firstRowOptions, LAYOUT_ROW, ALIGN_CENTER, JUSTIFY_CENTER);
        set_container_layout(secondRowOptions, LAYOUT_ROW, ALIGN_CENTER, JUSTIFY_CENTER);
        set_container_gap(firstRowOptions, 20);
        set_container_gap(secondRowOptions, 20);
        set_container_background_color(firstRowOptions, 0x111111);
        set_container_background_color(secondRowOptions, 0x111111);
        add_child_to_container_component(optionsRowContainer, firstRowOptions);
        add_child_to_container_component(optionsRowContainer, secondRowOptions);

        // Create the car option components
        for (int i = 0; i < 10; i++) {
            UIComponent *individualCarContainer = create_container_component(0, 0, 110, 150);
            if (!individualCarContainer) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            set_container_layout(individualCarContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
            set_container_background_color(individualCarContainer, 0x222222);
            set_container_padding(individualCarContainer, 40, 40, 40, 40);
            set_container_border_radius(individualCarContainer, 20);
            set_container_border(individualCarContainer, 4, 0xAA0000);
            set_container_hover_color(individualCarContainer, 0xAA0000);

            // Create the car image component
            char carText[10];
            snprintf(carText, sizeof(carText), "Car %d", i + 1);
            UIComponent *carOption = create_text_component(carText, gameFont, 0xFFFFFF);
            if (!carOption) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            add_child_to_container_component(individualCarContainer, carOption);

            // Add the car image to the container
            if (i < 5) {
                add_child_to_container_component(firstRowOptions, individualCarContainer);
            } else {
                add_child_to_container_component(secondRowOptions, individualCarContainer);
            }

            perform_container_layout(individualCarContainer);
            containers[i] = individualCarContainer;
        }
        perform_container_layout(firstRowOptions);
        perform_container_layout(secondRowOptions);

    perform_container_layout(optionsRowContainer);
    perform_container_layout(carContainer);

    // Create the back button
    backButton = create_container_component(30, 30, 40, 40);
    if (!backButton) {
        destroy_ui_component(carContainer);
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
        destroy_ui_component(carContainer);
        free(this);
        return NULL;
    }
    add_child_to_container_component(backButton, backText);

    perform_container_layout(backButton);
    this->backButton = backButton;

    return this;
}

void select_car_destroy(SelectCar *this) {
    this->base.destroy(&this->base);
}

void select_car_draw(SelectCar *this) {
    this->base.draw(&this->base);
    this->base.draw_mouse(&this->base);
}

void select_car_process_event(SelectCar *this, EventType event) {
    this->base.process_event(&this->base, event);
}

int select_car_get_selected_option(SelectCar *this) {
    return this->selectedOption;
}

CarSelection select_car_get_chosen_level(SelectCar *this) {
    return this->chosenLevel;
}

void select_car_reset_state(SelectCar *this) {
    this->selectedOption = -1;
    this->chosenLevel = CAR_START;
    this->base.draw(&this->base);
}

