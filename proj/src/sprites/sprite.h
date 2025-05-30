/**
 * @file sprite.h
 * @brief Provides functions and data structures to handle 2D sprites.
 *
 */

#ifndef SPRITE_H_
#define SPRITE_H_

#include <stdint.h>
#include <stdio.h>

#include "controller/video_card.h"
#include "macros.h"


/**
 * @struct Sprite
 * @brief Represents a 2D sprite loaded from an XPM image.
 *
 * Contains information about the sprite's image data, position, dimensions,
 * movement speeds, and rendering attributes.
 */
typedef struct {
    uint32_t *map;
    int x, y;
    int width, height;
    int xspeed, yspeed;
    uint8_t bytes_per_pixel;
    uint8_t *data;
} Sprite;

// Sprite Class Method Prototypes

/**
 * @brief Creates a sprite from an XPM image.
 *
 * @param pic XPM map representing the image.
 * @param x Initial x-position of the sprite.
 * @param y Initial y-position of the sprite.
 * @param xspeed Horizontal movement speed.
 * @param yspeed Vertical movement speed.
 * @return Pointer to a new Sprite instance, or NULL on failure.
 */
Sprite *sprite_create_xpm(xpm_map_t pic, int x, int y, int xspeed, int yspeed);

/**
 * @brief Destroys a sprite and frees its associated memory.
 *
 * @param this Pointer to the Sprite to destroy.
 */
void sprite_destroy(Sprite *this);

/**
 * @brief Draws the sprite at the specified position.
 *
 * @param this Pointer to the Sprite to draw.
 * @param x Screen x-coordinate to draw the sprite.
 * @param y Screen y-coordinate to draw the sprite.
 * @param has_transparent Check if there is transparency in the sprite.
 * @return 0 on success, non-zero on error.
 */
int sprite_draw_xpm(Sprite *this, int x, int y, bool has_transparent);

/**
 * @brief Draws a partial section of the sprite.
 *
 * @param this Pointer to the Sprite.
 * @param x Screen x-coordinate to draw.
 * @param y Screen y-coordinate to draw.
 * @param width Width of the partial section to draw.
 * @param height Height of the partial section to draw.
 * @param has_transparent Check if there is transparency.
 * @return 0 on success, non-zero on error.
 */
int sprite_draw_partial_xpm(Sprite *this, int x, int y, int width, int height, bool has_transparent);

/**
 * @brief Draws the sprite rotated around a local pivot point.
 *
 * @param sprite Pointer to the Sprite.
 * @param screen_pivot_x X-coordinate on the screen to rotate around.
 * @param screen_pivot_y Y-coordinate on the screen to rotate around.
 * @param local_pivot_x X pivot within the sprite.
 * @param local_pivot_y Y pivot within the sprite.
 * @param cos_a Cosine of the rotation angle.
 * @param sin_a Sine of the rotation angle.
 * @param has_transparent Check if there is transparency.
 * @return 0 on success, non-zero on error.
 */
int sprite_draw_rotated_around_local_pivot( Sprite *sprite, int screen_pivot_x, int screen_pivot_y, int local_pivot_x, int local_pivot_y, float cos_a, float sin_a, bool has_transparent);

// Getters and Setters (if needed)
/**
 * @brief Gets the width of the sprite.
 *
 * @param this Pointer to the Sprite.
 * @return Width in pixels.
 */
int sprite_get_width(Sprite *this);

/**
 * @brief Gets the height of the sprite.
 *
 * @param this Pointer to the Sprite.
 * @return Height in pixels.
 */
int sprite_get_height(Sprite *this);

/**
 * @brief Gets the x-coordinate of the sprite.
 *
 * @param this Pointer to the Sprite.
 * @return X-position on the screen.
 */
int sprite_get_x(Sprite *this);

/**
 * @brief Gets the y-coordinate of the sprite.
 *
 * @param this Pointer to the Sprite.
 * @return Y-position on the screen.
 */
int sprite_get_y(Sprite *this);

/**
 * @brief Sets the x-coordinate of the sprite.
 *
 * @param this Pointer to the Sprite.
 * @param x New x-position.
 */
void sprite_set_x(Sprite *this, int x);

/**
 * @brief Sets the y-coordinate of the sprite.
 *
 * @param this Pointer to the Sprite.
 * @param y New y-position.
 */
void sprite_set_y(Sprite *this, int y);

/**
 * @brief Moves the sprite by a delta in both x and y directions.
 *
 * @param this Pointer to the Sprite.
 * @param dx Delta x (horizontal movement).
 * @param dy Delta y (vertical movement).
 */
void sprite_move(Sprite *this, int dx, int dy);

#endif /* SPRITE_H_ */
