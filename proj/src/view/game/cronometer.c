#include <lcom/lcf.h>
#include "cronometer.h"
#include "game.h"

extern Font *gameFont;
extern Game *game;

static UIComponent *currentTime = NULL;
static UIComponent *currentTimeContainer = NULL;
static UIComponent *currentTimeText = NULL;

UIComponent *display_cronometer(float cronometer_time) {
  char timer_string[16];
  int minutes = (int)(cronometer_time) / 60;
  int seconds = (int)(cronometer_time) % 60;
  int miliseconds = (int)((cronometer_time - (int)cronometer_time) * 100);
  sprintf(timer_string, "%02d:%02d.%02d", minutes, seconds, miliseconds);

  UIComponent *timerText = create_text_component(timer_string, gameFont, 0xFFFFFF);
  if (timerText && timerText->data) {
    timerText->x = 20;
    timerText->y = 20;
  }

  return timerText;
}

float get_current_cronometer_time() {
  if (game) {
    return game->cronometer_time;
  }
  return 0.0f;
}


UIComponent *display_current_time(UIComponent *parent) {

  currentTime = display_cronometer(get_current_cronometer_time());
  currentTimeContainer = create_container_component(0, 0, 0, 0);
  
  set_container_layout(currentTimeContainer, LAYOUT_ROW, ALIGN_CENTER, JUSTIFY_CENTER);
  set_container_background_color(currentTimeContainer, 0x1C1C1C);
  set_container_padding(currentTimeContainer, 15, 15, 15, 15);
  set_container_border(currentTimeContainer, 2, 0xFFDD00); 
  set_container_border_radius(currentTimeContainer, 8);
  set_container_gap(currentTimeContainer, 10); 

  currentTimeText = create_text_component("Current Time: ", gameFont, 0xFFFFFF);
  if (!currentTimeText) {
    destroy_ui_component(parent);
    return NULL;
  }

  add_child_to_container_component(currentTimeContainer, currentTimeText);
  add_child_to_container_component(currentTimeContainer, currentTime);
  add_child_to_container_component(parent, currentTimeContainer);

  perform_container_layout(parent);

  return currentTimeContainer;
}

void update_current_time_display() {
  if (currentTime && currentTimeContainer) {
    UIComponent *newTime = display_cronometer(get_current_cronometer_time());
    if (newTime) {
      destroy_ui_component(currentTime);
      currentTime = newTime;
      ContainerData *data = (ContainerData *)currentTimeContainer->data;
      if (data && data->num_children >= 2) {
        data->children[1] = currentTime;
        perform_container_layout(currentTimeContainer);
      }
    }
  }
}

