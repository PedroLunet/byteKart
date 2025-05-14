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
static UIComponent *firstCarContainer = NULL;
static UIComponent *secondCarContainer = NULL;
static UIComponent *thirdCarContainer = NULL;
static UIComponent *fourthCarContainer = NULL;
static UIComponent *firstCarOption = NULL;
static UIComponent *secondCarOption = NULL;
static UIComponent *thirdCarOption = NULL;
static UIComponent *fourthCarOption = NULL;

static void select_car_draw_internal(GameState *base) {
    SelectCar *this = (SelectCar *)base;
    if (this->uiRoot) {
        draw_ui_component(this->uiRoot);
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

              if (optionsRowContainerData->num_children >= 2) {
                  UIComponent *firstRowConteiner = optionsRowContainerData->children[0];
                  UIComponent *secondRowContainer = optionsRowContainerData->children[1];

                  if (firstRowConteiner && firstRowConteiner->type == TYPE_CONTAINER && firstRowConteiner->data) {
                      ContainerData *firstRowData = (ContainerData *)firstRowConteiner->data;

                      if (firstRowData->num_children >= 2) {
                            UIComponent *firstCarContainer = firstRowData->children[0];
                            UIComponent *secondCarContainer = firstRowData->children[1];

                            if (firstCarContainer && firstCarContainer->type == TYPE_CONTAINER && firstCarContainer->data) {
                                ContainerData *firstCarData = (ContainerData *)firstCarContainer->data;
                                if (mouse_x >= firstCarContainer->x && mouse_x < firstCarContainer->x + firstCarData->width &&
                                    mouse_y >= firstCarContainer->y && mouse_y < firstCarContainer->y + firstCarData->height) {
                                    *selected = 0;
                                    set_container_background_color(firstCarContainer, 0xA81D1D);
                                    return true;
                                }
                            }

                            if (secondCarContainer && secondCarContainer->type == TYPE_CONTAINER && secondCarContainer->data) {
                                ContainerData *secondCarData = (ContainerData *)secondCarContainer->data;
                                if (mouse_x >= secondCarContainer->x && mouse_x < secondCarContainer->x + secondCarData->width &&
                                    mouse_y >= secondCarContainer->y && mouse_y < secondCarContainer->y + secondCarData->height) {
                                    *selected = 1;
                                    set_container_background_color(secondCarContainer, 0xA81D1D);
                                    return true;
                                }
                            }

                            if (*selected != 0) {
                                set_container_background_color(firstCarContainer, 0x111111);
                            }
                            if (*selected != 1) {
                                set_container_background_color(secondCarContainer, 0x111111);
                            }
                      }
                  }

                  if (secondRowContainer && secondRowContainer->type == TYPE_CONTAINER && secondRowContainer->data) {
                      ContainerData *secondRowData = (ContainerData *)secondRowContainer->data;

                      if (secondRowData->num_children >= 2) {
                            UIComponent *thirdCarContainer = secondRowData->children[0];
                            UIComponent *fourthCarContainer = secondRowData->children[1];

                            if (thirdCarContainer && thirdCarContainer->type == TYPE_CONTAINER && thirdCarContainer->data) {
                                ContainerData *thirdCarData = (ContainerData *)thirdCarContainer->data;
                                if (mouse_x >= thirdCarContainer->x && mouse_x < thirdCarContainer->x + thirdCarData->width &&
                                    mouse_y >= thirdCarContainer->y && mouse_y < thirdCarContainer->y + thirdCarData->height) {
                                    *selected = 2;
                                    set_container_background_color(thirdCarContainer, 0xA81D1D);
                                    return true;
                                }
                            }

                            if (fourthCarContainer && fourthCarContainer->type == TYPE_CONTAINER && fourthCarContainer->data) {
                                ContainerData *fourthCarData = (ContainerData *)fourthCarContainer->data;
                                if (mouse_x >= fourthCarContainer->x && mouse_x < fourthCarContainer->x + fourthCarData->width &&
                                    mouse_y >= fourthCarContainer->y && mouse_y < fourthCarContainer->y + fourthCarData->height) {
                                    *selected = 3;
                                    set_container_background_color(fourthCarContainer, 0xA81D1D);
                                    return true;
                                }
                            }

                            // Reset background color for unselected options
                            if (*selected != 2) {
                                set_container_background_color(thirdCarContainer, 0x111111);
                            }
                            if (*selected != 3) {
                                set_container_background_color(fourthCarContainer, 0x111111);
                            }
                      }
                  }
              }
          }

    }

    return false;
}

static void select_car_process(GameState *base, EventType event) {
    SelectCar *this = (SelectCar *)base;
    if (event == EVENT_KEYBOARD) {
        switch (scancode) {
            case UP_ARROW:
                if (this->selectedOption > 0) this->selectedOption--;
            break;
            case DOWN_ARROW:
                if (this->selectedOption < 4) this->selectedOption++;
            break;
            case ENTER_KEY:
                if (this->selectedOption == 0) this->chosenLevel = CAR_FIRST;
                else if (this->selectedOption == 1) this->chosenLevel = CAR_SECOND;
                else if (this->selectedOption == 2) this->chosenLevel = CAR_THIRD;
                else if (this->selectedOption == 3) this->chosenLevel = CAR_FOURTH;
            break;
            case ESC_BREAKCODE:
                this->chosenLevel = CAR_EXITED;
            break;
            default:
                break;
        }
        base->draw(base);
    } else if (event == EVENT_MOUSE) {
        int prevSelected = this->selectedOption;
        if (base->handle_mouse_input(base, (void (*)(GameState *))select_car_draw, select_car_is_mouse_over, &this->selectedOption)) {
            if (this->selectedOption != -1) {
                if (this->selectedOption == 0) this->chosenLevel = CAR_FIRST;
                else if (this->selectedOption == 1) this->chosenLevel = CAR_SECOND;
                else if (this->selectedOption == 2) this->chosenLevel = CAR_THIRD;
                else if (this->selectedOption == 3) this->chosenLevel = CAR_FOURTH;
            }
        }
        if (this->selectedOption != prevSelected) {
            base->draw(base);
        }
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
        firstCarContainer = NULL;
        secondCarContainer = NULL;
        thirdCarContainer = NULL;
        fourthCarContainer = NULL;
        firstCarOption = NULL;
        secondCarOption = NULL;
        thirdCarOption = NULL;
        fourthCarOption = NULL;
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

    this->selectedOption = 0;
    this->chosenLevel = CAR_START;
    this->uiRoot = NULL;

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
        if (!firstRowOptions) {
            destroy_ui_component(carContainer);
            free(this);
            return NULL;
        }
        set_container_layout(firstRowOptions, LAYOUT_ROW, ALIGN_CENTER, JUSTIFY_CENTER);
        set_container_gap(firstRowOptions, 20);
        set_container_background_color(firstRowOptions, 0x111111);
        add_child_to_container_component(optionsRowContainer, firstRowOptions);

            // Create the first car option
            firstCarContainer = create_container_component(0, 0, 150, 170);
            if (!firstCarContainer) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            set_container_layout(firstCarContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
            set_container_background_color(firstCarContainer, 0xAA0000);
            set_container_padding(firstCarContainer, 40, 40, 40, 40);
            set_container_border_radius(firstCarContainer, 20);
            set_container_border(firstCarContainer, 4, 0x8F8483);
            // Sprite *red_car = sprite_create_xpm((xpm_map_t) red_car_xpm, 0, 0, 0, 0);
            // firstCarOption = create_sprite_component(red_car, 0, 0);
            firstCarOption = create_text_component("Car 1", gameFont, 0xFFFFFF);
            if (!firstCarOption) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            add_child_to_container_component(firstCarContainer, firstCarOption);
            add_child_to_container_component(firstRowOptions, firstCarContainer);
            perform_container_layout(firstCarContainer);

            // Create the second car option
            secondCarContainer = create_container_component(0, 0, 150, 170);
            if (!secondCarContainer) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            set_container_layout(secondCarContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
            set_container_background_color(secondCarContainer, 0xAA0000);
            set_container_padding(secondCarContainer, 40, 40, 40, 40);
            set_container_border_radius(secondCarContainer, 20);
            set_container_border(secondCarContainer, 4, 0x8F8483);
            // Sprite *pink_car = sprite_create_xpm((xpm_map_t) pink_car_xpm, 0, 0, 0, 0);
            //secondCarOption = create_sprite_component(pink_car, 0, 0);
            secondCarOption = create_text_component("Car 2", gameFont, 0xFFFFFF);
            if (!secondCarOption) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            add_child_to_container_component(secondCarContainer, secondCarOption);
            add_child_to_container_component(firstRowOptions, secondCarContainer);
            perform_container_layout(secondCarContainer);

        perform_container_layout(firstRowOptions);


        // Create the second row of car options
        secondRowOptions = create_container_component(0, 0, 0, 0);
        if (!secondRowOptions) {
            destroy_ui_component(carContainer);
            free(this);
            return NULL;
        }
        set_container_layout(secondRowOptions, LAYOUT_ROW, ALIGN_CENTER, JUSTIFY_CENTER);
        set_container_gap(secondRowOptions, 20);
        set_container_background_color(secondRowOptions, 0x111111);
        add_child_to_container_component(optionsRowContainer, secondRowOptions);

            // Create the third car option
            thirdCarContainer = create_container_component(0, 0, 150, 170);
            if (!thirdCarContainer) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            set_container_layout(thirdCarContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
            set_container_background_color(thirdCarContainer, 0xAA0000);
            set_container_padding(thirdCarContainer, 40, 40, 40, 40);
            set_container_border_radius(thirdCarContainer, 20);
            set_container_border(thirdCarContainer, 4, 0x8F8483);
            // Sprite *orange_car = sprite_create_xpm((xpm_map_t) orange_car_xpm, 0, 0, 0, 0);
            //thirdCarOption = create_sprite_component(orange_car, 0, 0);
            thirdCarOption = create_text_component("Car 3", gameFont, 0xFFFFFF);
            if (!thirdCarOption) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            add_child_to_container_component(thirdCarContainer, thirdCarOption);
            add_child_to_container_component(secondRowOptions, thirdCarContainer);
            perform_container_layout(thirdCarContainer);

            // Create the fourth car option
            fourthCarContainer = create_container_component(0, 0, 150, 170);
            if (!fourthCarContainer) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            set_container_layout(fourthCarContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
            set_container_background_color(fourthCarContainer, 0xAA0000);
            set_container_padding(fourthCarContainer, 40, 40, 40, 40);
            set_container_border_radius(fourthCarContainer, 20);
            set_container_border(fourthCarContainer, 4, 0x8F8483);
            // Sprite *blue_car = sprite_create_xpm((xpm_map_t) blue_car_xpm, 0, 0, 0, 0);
            // fourthCarOption = create_sprite_component(blue_car, 0, 0);
            fourthCarOption = create_text_component("Car 4", gameFont, 0xFFFFFF);
            if (!fourthCarOption) {
                destroy_ui_component(carContainer);
                free(this);
                return NULL;
            }
            add_child_to_container_component(fourthCarContainer, fourthCarOption);
            add_child_to_container_component(secondRowOptions, fourthCarContainer);
            perform_container_layout(fourthCarContainer);
        perform_container_layout(secondRowOptions);
    perform_container_layout(optionsRowContainer);

    // Add the rows to the options row container
    perform_container_layout(carContainer);

    return this;
}

void select_car_destroy(SelectCar *this) {
    this->base.destroy(&this->base);
}

void select_car_draw(SelectCar *this) {
    this->base.clear_mouse_area(&this->base);
    this->base.draw(&this->base);
    this->base.draw_mouse(&this->base);
}

void select_car_process_event(SelectCar *this, EventType event) {
    this->base.process_event(&this->base, event);
}

CarSelection select_car_get_chosen_level(SelectCar *this) {
    return this->chosenLevel;
}

void select_car_reset_state(SelectCar *this) {
    this->selectedOption = 0;
    this->chosenLevel = CAR_START;
    this->base.draw(&this->base);
}

