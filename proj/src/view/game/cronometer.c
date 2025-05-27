#include <lcom/lcf.h>
#include "cronometer.h"

extern Font *gameFont;

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
    draw_ui_component(timerText);
    destroy_ui_component(timerText);
  }

  return timerText;
}

