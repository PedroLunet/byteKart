/**
 * @file car.h
 * @brief Defines a simple car structure and movement behavior.
 *
 */
#ifndef CAR_H
#define CAR_H

#include "macros.h"
#include "sprites/sprite.h"

#define STEP 10 /**< Default movement step size for car movement. */

/**
 * @struct Car
 * @brief Represents a car with position, speed, and a visual sprite.
 */
typedef struct Car {
    int x, y;
    int speed;
    Sprite *car_sprite;
} Car;

/**
 * @brief Moves the car to the left by a given step, with a minimum boundary check.
 *
 * @param car Pointer to the Car instance.
 * @param min Minimum X boundary to prevent going off-screen.
 * @param step Distance to move left.
 */
void car_move_left(Car *car, int min, int step);

/**
 * @brief Moves the car to the right by a given step, with a maximum boundary check.
 *
 * @param car Pointer to the Car instance.
 * @param max Maximum X boundary to prevent going off-screen.
 * @param step Distance to move right.
 */
void car_move_right(Car *car, int max, int step);

#endif

