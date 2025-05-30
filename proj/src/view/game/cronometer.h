/**
 * @file cronometer.h
 * @brief Provides utilities for displaying and managing a race timer (cronometer).
 *
 */

#ifndef CRONOMETER_H
#define CRONOMETER_H

#include "../utils/ui.h"
#include "../../fonts/font.h"

/**
 * @brief Displays a static cronometer (race timer) component.
 *
 * Creates and returns a UI component showing the given time.
 *
 * @param cronometer_time Time to be displayed in seconds.
 * @return Pointer to the created UIComponent.
 */
UIComponent *display_cronometer(float cronometer_time);

/**
 * @brief Returns the current value of the cronometer.
 *
 * @return Current cronometer time in seconds.
 */
float get_current_cronometer_time();

/**
 * @brief Creates and attaches current timer of the game to a parent component.
 *
 * @param parent Pointer to the parent UIComponent to attach to.
 * @return Pointer to the UIComponent displaying current time.
 */
UIComponent *display_current_time(UIComponent *parent);

/**
 * @brief Updates the real-time cronometer UI with the latest time.
 *
 * When called, update the current time of the game displayed.
 */
void update_current_time_display();

#endif

