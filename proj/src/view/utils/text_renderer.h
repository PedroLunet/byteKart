/**
 * @file text_renderer.h
 * @brief Provides functions for rendering text using bitmap fonts and handling keyboard input mapping.
 *
 */
#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <stdio.h>
#include <stdint.h>

#include <stdbool.h>
#include "../../fonts/font.h"
#include "../../sprites/sprite.h"
#include "controller/video_card.h"


/**
 * @brief Renders a string of text into a pixel buffer using a specified font.
 *
 * Each character is drawn from the font atlas using glyph metadata. The pixel data buffer
 * should be preallocated and match the desired screen or UI component resolution.
 *
 * @param text The null-terminated string to render.
 * @param x Starting X position in pixels.
 * @param y Starting Y position in pixels.
 * @param color The color to render the text.
 * @param font Pointer to the Font containing glyph data and atlas texture.
 * @param pixel_data Pointer to the destination pixel buffer.
 * @param buffer_width Width of the pixel buffer (in pixels).
 * @return 0 on success, negative value on failure.
 */
int load_text(const char *text, int x, int y, uint32_t color, const Font *font, uint32_t *pixel_data, int buffer_width);

/**
 * @brief Converts a keyboard scancode to its corresponding ASCII character.
 *
 * This function maps basic scancodes into ASCII.
 * It is useful for basic text input fields or name entry systems.
 *
 * @param scancode Keyboard scancode to convert.
 * @return Corresponding ASCII character, or 0 if unmapped.
 */
char scancode_to_char(uint8_t scancode);

#endif
