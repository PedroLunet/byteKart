#ifndef ITEMS_H
#define ITEMS_H

#include "model/geometry.h"
#include "model/sprite.h"
#include "model/obb.h"
#include "model/physics_utils.h"
#include "model/ai_car.h"
#include "model/player.h"
#include "xpm/xpm_files.h"
#include "view/renderer.h"
#include "macros.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
    OBSTACLE_TYPE_BARRIER,
    OBSTACLE_TYPE_OIL_SLICK,
    // Add more obstacle types
} ObstacleType;

typedef enum {
    POWERUP_TYPE_NONE = 0,
    POWERUP_TYPE_SPEED_BOOST,
    POWERUP_TYPE_SHIELD,
    // POWERUP_TYPE_PROJECTILE,
    // Add more power-up types
    POWERUP_TYPE_COUNT
} PowerUpType;

// --- Item Structures ---
typedef struct {
    ObstacleType type;
    Point world_position;
    Sprite *sprite;
    OBB obb;
    bool is_active;
    float hitbox_half_width;
    float hitbox_half_height;
} Obstacle;

typedef struct {
    Point world_position;
    Sprite *sprite_active;
    OBB obb;
    bool is_active;
    float respawn_timer_s;
    float hitbox_half_width;
    float hitbox_half_height;
} PowerUpBox;

int items_init(GameItems *items, const Road* road);
void items_destroy(GameItems *items);
void items_update(GameItems *items, Player *player, AICar *ai_cars[], int num_ai_cars, const Road *road, float delta_time);
void items_draw(const GameItems *items, const Player *player_view);
PowerUpType items_assign_random_powerup(bool car_is_player, int car_rank, int total_racers);

void items_apply_speed_boost(void* car_entity, bool is_player, float duration, float factor);
void items_apply_shield(void* car_entity, bool is_player, float duration);

#endif //ITEMS_H
