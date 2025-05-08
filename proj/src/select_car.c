#include <lcom/lcf.h>
#include <stdlib.h>

#include "select_car.h"
#include "macros.h"
#include "xpm/xpm_files.h"
#include "controller/video_card.h"
#include "controller/mouse.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern uint8_t index_packet;
extern struct packet pp;

SelectCar *select_car_create() {
    SelectCar *this = (SelectCar *) malloc(sizeof(SelectCar));
    if (this == NULL) {
        return NULL;
    }

    this->car1Sprite = sprite_create_xpm((xpm_map_t) car1, 0, 0, 0, 0);
    if (this->car1Sprite == NULL) {
        free(this);
        return NULL;
    }

    this->car2Sprite = sprite_create_xpm((xpm_map_t) car2, 0, 0, 0, 0);
    if (this->car2Sprite == NULL) {
        sprite_destroy(this->car1Sprite);
        free(this);
        return NULL;
    }

    this->car3Sprite = sprite_create_xpm((xpm_map_t) car3, 0, 0, 0, 0);
    if (this->car3Sprite == NULL) {
        sprite_destroy(this->car1Sprite);
        sprite_destroy(this->car2Sprite);
        free(this);
        return NULL;
    }

    this->backSprite = sprite_create_xpm((xpm_map_t) back, 0, 0, 0, 0);
    if (this->backSprite == NULL) {
        sprite_destroy(this->car1Sprite);
        sprite_destroy(this->car2Sprite);
        sprite_destroy(this->car3Sprite);
        free(this);
        return NULL;
    }

    this->centerX = (vbe_mode_info.XResolution) / 2;
    this->centerY = (vbe_mode_info.YResolution) / 2;
    this->mouse_x = (int)vbe_mode_info.XResolution / 2;
    this->mouse_y = (int)vbe_mode_info.YResolution / 2;
    this->currentSubstate = SELECT_CAR_CAR1;

    return this;
}

void select_car_destroy(SelectCar *this) {
    if (this) {
        sprite_destroy(this->car1Sprite);
        sprite_destroy(this->car2Sprite);
        sprite_destroy(this->car3Sprite);
        sprite_destroy(this->backSprite);
        free(this);
    }
}

void select_car_process_event(SelectCar *this, EventType event) {
    if (event == EVENT_KEYBOARD) {
        switch (scancode) {

        }
        menu_draw(this);
    } else if (event == EVENT_MOUSE) {
        if (handle_mouse_input(this)) {

        }
    }
}

SelectCarSubstate select_car_get_current_substate(SelectCar *this) {
    return this->currentSubstate;
}

void select_car_reset_state(SelectCar *this) {
    this->currentSubstate = SELECT_CAR_CAR1;
}
