/**
 * @file loadingUI.h
 * @brief Declares the LoadingUI struct and its related UI handling functions.
 *
 */
#ifndef LOADINGUI_H
#define LOADINGUI_H

#include "view/utils/ui.h"
#include "controller/video_card.h"
#include <stdint.h>


/**
 * @struct LoadingUI
 * @brief Represents a basic UI shown during loading screens.
 *
 * Contains a fixed array of UI components.
 */
typedef struct {
    UIComponent *components[3]; /**< Array of UI components used in the loading screen. */
} LoadingUI;

/**
 * @brief Creates and initializes a new LoadingUI instance.
 *
 * @param font Pointer to the Font used for rendering loading text.
 * @param width Width of the screen or UI area.
 * @param height Height of the screen or UI area.
 * @return Pointer to a newly created LoadingUI.
 */
LoadingUI *loading_ui_create(Font *font, uint16_t width, uint16_t height);

/**
 * @brief Swaps the video buffer for rendering the loading screen.
 *
 * This should be called after updating/loading content to visually reflect progress.
 */
void swap_buffer_loading_ui();

/**
 * @brief Frees all resources associated with the given LoadingUI.
 *
 * @param ui Pointer to the LoadingUI instance to destroy.
 */
void loading_ui_destroy(LoadingUI *ui);

#endif //LOADINGUI_H
