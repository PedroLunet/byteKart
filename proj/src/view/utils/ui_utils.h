/**
 * @file ui_utils.h
 * @brief Utility functions for creating and managing UI components such as buttons, titles, and containers.
 * 
 */
#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <stdlib.h>

#include "ui.h"
#include "fonts/font.h" 
#include "model/game_state.h"

/**
 * @brief Creates a styled menu option component.
 *
 * @param text Label text for the menu option.
 * @param font Pointer to the font used to render the text.
 * @param width Desired width of the option component.
 * @param height Desired height of the option component.
 * @param parent Parent UIComponent to attach this to.
 * @return Pointer to the created UIComponent.
 */
UIComponent *create_menu_option(const char *text, Font *font, int width, int height, UIComponent *parent);

/**
 * @brief Detects if the mouse is hovering over any menu options.
 *
 * Updates the selected option based on hover state and changes option colors accordingly.
 *
 * @param base Pointer to the base GameState for context.
 * @param mouse_x Current mouse X position.
 * @param mouse_y Current mouse Y position.
 * @param options Array of UIComponent pointers representing menu options.
 * @param num_options Number of elements in the options array.
 * @param selected_option Pointer to store the index of the currently selected option.
 * @param default_color Default background/text color.
 * @param hover_color Highlight color used on hover.
 * @return true if the mouse is over any option; false otherwise.
 */
bool is_mouse_over_menu_options(GameState *base, int mouse_x, int mouse_y, UIComponent **options, size_t num_options, int *selected_option, uint32_t default_color, uint32_t hover_color);

/**
 * @brief Creates a standardized "Back" button UI component.
 *
 * @param font Pointer to the font used for the button label.
 * @param parent Parent component to attach the button to.
 * @return Pointer to the created back button.
 */
UIComponent *create_back_button(Font *font, UIComponent *parent);

/**
 * @brief Creates a title label for use in menus and overlays.
 *
 * @param title The title text to display.
 * @param font Pointer to the font used to render the text.
 * @param color Color of the title text.
 * @param parent UI parent component.
 * @return Pointer to the title UIComponent.
 */
UIComponent *create_title_text(const char *title, Font *font, uint32_t color, UIComponent *parent);

/**
 * @brief Creates a main UI container component with padding and background.
 *
 * @param background Optional background sprite.
 * @param gap Gap between child elements.
 * @param top_pad Padding from the top edge.
 * @param bot_pad Padding from the bottom edge.
 * @param left_pad Padding from the left edge.
 * @param right_pad Padding from the right edge.
 * @return Pointer to the main container UIComponent.
 */
UIComponent *create_main_container(Sprite *background, int gap, int top_pad, int bot_pad, int left_pad, int right_pad);

/**
 * @brief Creates a row layout container for horizontally arranging options.
 *
 * @param gap Gap between children in the row.
 * @param layout Layout direction or flags.
 * @param parent Parent UI component.
 * @return Pointer to the row layout UIComponent.
 */
UIComponent *create_row_options(int gap, int layout, UIComponent *parent);

/**
 * @brief Checks whether the mouse is hovering over the back button.
 *
 * Updates selected option index if the back button is hovered.
 *
 * @param backButton Pointer to the back button UIComponent.
 * @param mouse_x Current X position of the mouse.
 * @param mouse_y Current Y position of the mouse.
 * @param selected_option Pointer to an int storing the selected option.
 * @param option Value to set selected_option to if hovered.
 * @return true if the back button is hovered; false otherwise.
 */
bool is_mouse_over_back_button(UIComponent *backButton, int mouse_x, int mouse_y, int *selected_option, int option);

#endif

