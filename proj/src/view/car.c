#include <lcom/lcf.h>

#include "car.h"

void car_move_left(Car *car, int min, int step) {
  if (car->x > min) {
    car->x -= step;
  }
}

void car_move_right(Car *car, int max, int step) {
  if (car->x < max) {
    car->x += step;
  }
}

