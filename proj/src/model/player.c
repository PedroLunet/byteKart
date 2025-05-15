#include <lcom/lcf.h>
#include "player.h"

extern vbe_mode_info_t vbe_mode_info;

static float precalculated_cos_turn;
static float precalculated_sin_turn;

void create_player(Player *this, Point *center, Vector *direction) {
    this->position.x = center->x - (vbe_mode_info.XResolution / 2);
    this->position.y = center->y - (vbe_mode_info.YResolution / 2);
    this->width = vbe_mode_info.XResolution;
    this->height = vbe_mode_info.YResolution;
    this->speed = 0;
    this->direction.x = direction->x;
    this->direction.y = direction->y;
    this->velocity.x = 0.0f;
    this->velocity.y = 0.0f;

    precalculated_cos_turn = cos(PLAYER_TURN_INCREMENT_RAD);
    precalculated_sin_turn = sin(PLAYER_TURN_INCREMENT_RAD);
}

void player_movement(Player *this) {
    this->position.x += this->velocity.x;
    this->position.y += this->velocity.y;
}

void player_set_speed(Player *this, float speed) {
    this->speed = speed;
    this->velocity.x = this->speed * this->direction.x;
    this->velocity.y = this->speed * this->direction.y;
}

void player_turn(Player *this, int turn_direction_sign) {
    float old_dx = this->direction.x;
    float old_dy = this->direction.y;

    float effective_sin = turn_direction_sign * precalculated_sin_turn;
    float effective_cos = precalculated_cos_turn;

    this->direction.x = old_dx * effective_cos - old_dy * effective_sin;
    this->direction.y = old_dx * effective_sin + old_dy * effective_cos;

    vector_normalize(&this->direction);
    this->velocity.x = this->speed * this->direction.x;
    this->velocity.y = this->speed * this->direction.y;
}
