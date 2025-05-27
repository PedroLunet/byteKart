#ifndef CRONOMETER_H
#define CRONOMETER_H

#include "../utils/ui.h"
#include "../../fonts/font.h"

UIComponent *display_cronometer(float cronometer_time);
float get_current_cronometer_time();
UIComponent *display_current_time(UIComponent *parent);

#endif

