#include <lcom/lcf.h>

#include "select_car.h"

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

static UIComponent *containers[6];

const xpm_map_t car_choices[6] = {
    (xpm_map_t) pink_car_xpm,
    (xpm_map_t) black_car_xpm,
    (xpm_map_t) red_car_xpm,
    (xpm_map_t) orange_car_xpm,
    (xpm_map_t) blue_car_xpm,
    (xpm_map_t) green_car_xpm
};

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

              for (int row = 0; row < 3; row++) {
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
                                     *selected = row * 3 + col;
                                     return true;
                                 }
                              }
                          }
                      }
                  }
              }
          }

    }

    if (is_mouse_over_back_button(this->backButton, mouse_x, mouse_y, selected, 6))
        return true;
    
    return false;
}

static void select_car_process(GameState *base, EventType event) {
    SelectCar *this = (SelectCar *)base;
    int prevSelected = this->selectedOption;
    if (event == EVENT_MOUSE) {
        if (base->handle_mouse_input(base, (void (*)(GameState *))select_car_clean_dirty_mouse_internal, select_car_is_mouse_over, &this->selectedOption)) {
            this->chosenLevel = CAR_SELECTED;
        }
    }

    if (this->selectedOption >= 0 && this->selectedOption < 6 && containers[this->selectedOption] != NULL) {
        is_container_hovered(containers[this->selectedOption]);
    } else if (this->selectedOption == 6 && backButton != NULL) {
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

    // Main Container
    UIComponent *carContainer = create_main_container(NULL, 30, 0, 0, 60, 60);
    this->uiRoot = carContainer;

    // Create the title text component
    titleText = create_title_text("Select your car", gameFont, 0xFFFFFF, carContainer);

    // Create a container for the car options in a row
    optionsRowContainer = create_row_options(20, LAYOUT_COLUMN, carContainer);

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
        for (int i = 0; i < 6; i++) {
            UIComponent *individualCarContainer = create_container_component(0, 0, 0, 0);
            if (!individualCarContainer) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            set_container_layout(individualCarContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
            set_container_background_color(individualCarContainer, 0x222222);
            set_container_padding(individualCarContainer, 20, 20, 20, 20);
            set_container_border_radius(individualCarContainer, 20);
            set_container_border(individualCarContainer, 4, 0xAA0000);
            set_container_hover_color(individualCarContainer, 0xAA0000);

            // Create the car image component
            Sprite *carSprite = sprite_create_xpm(car_choices[i], 0, 0, 0, 0);
            UIComponent *carChoice = create_sprite_component(carSprite, 0, 0);
            if (!carChoice) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            add_child_to_container_component(individualCarContainer, carChoice);

            // Add the car image to the container
            if (i < 3) {
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

    // Create the back button
    backButton = create_back_button(gameFont, carContainer);
    
    perform_container_layout(carContainer);
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

