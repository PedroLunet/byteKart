#ifndef SELECT_CAR_H
#define SELECT_CAR_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "model/game_state.h"
#include "xpm/xpm_files.h"
#include "view/utils/ui.h"
#include "macros.h"
#include "controller/video_card.h"
#include "controller/mouse.h"
#include "view/utils/ui_utils.h"

extern const xpm_map_t car_choices[6];

typedef enum {
    CAR_START,
    CAR_SELECTED,
    CAR_EXITED
} CarSelection;

typedef struct SelectCar {
    GameState base;
    int selectedOption;
    CarSelection chosenLevel;
    UIComponent *uiRoot;
    UIComponent *backButton;
} SelectCar;

SelectCar *select_car_create();
void select_car_destroy(SelectCar *this);
void select_car_draw(SelectCar *this);
void select_car_process_event(SelectCar *this, EventType event);
int select_car_get_selected_option(SelectCar *this);
CarSelection select_car_get_chosen_level(SelectCar *this);
void select_car_reset_state(SelectCar *this);

#endif
