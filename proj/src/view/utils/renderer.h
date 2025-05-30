/**
 * @file renderer.h
 * @brief Provides functions for 2D/3D rendering of game elements including the player, AI cars, and road.
 *
 */
#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#include "controller/video_card.h"
#include "model/geometry.h"
#include "sprites/sprite.h"
#include "model/player.h"
#include "model/ai_car.h"
#include "view/game/road.h"

/**
 * @struct Point_i
 * @brief Represents a 2D integer-based screen point.
 */
typedef struct {
  int x; /**< X coordinate on screen. */
  int y; /**< Y coordinate on screen. */
} Point_i;

/**
 * @struct TexturedVertex
 * @brief Represents a vertex with screen position and texture coordinates.
 */
typedef struct {
  Point_i screen_pos; /**< Screen position of the vertex. */
  float u;            /**< U texture coordinate. */
  float v;            /**< V texture coordinate. */
} TexturedVertex;

/**
 * @brief Initializes the rendering system and prepares internal buffers.
 * @return 0 on success, non-zero on failure.
 */
int renderer_init();

/**
 * @brief Presents the current rendering buffer to the screen.
 */
void renderer_present_buffer();

/**
 * @brief Clears the rendering buffer with a given background color.
 * @param color The color to fill the buffer with.
 */
void renderer_clear_buffer(uint32_t color);

/**
 * @brief Transforms a world-space coordinate to screen-space relative to the player's view.
 * @param player_view Pointer to the player struct (used for camera position).
 * @param world_pos World position to be transformed.
 * @param screen_pos Output screen position.
 */
void renderer_transform_world_to_screen(const Player *player_view, Point world_pos, Point_i *screen_pos);

/**
 * @brief Draws a single pixel at the specified screen location.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param color Color of the pixel.
 */
void renderer_draw_pixel(int x, int y, uint32_t color);

/**
 * @brief Draws a line between two points using Bresenham’s algorithm.
 * @param x1 Start point X.
 * @param y1 Start point Y.
 * @param x2 End point X.
 * @param y2 End point Y.
 * @param color Color of the line.
 */
void renderer_draw_line(int x1, int y1, int x2, int y2, uint32_t color);

/**
 * @brief Draws a filled rectangle on the screen.
 * @param x X coordinate of the top-left corner.
 * @param y Y coordinate of the top-left corner.
 * @param width Width of the rectangle.
 * @param height Height of the rectangle.
 * @param color Fill color.
 */
void renderer_draw_rectangle_filled(int x, int y, int width, int height, uint32_t color);

/**
 * @brief Draws a textured triangle using screen coordinates and texture UVs.
 * @param v1 First vertex of the triangle.
 * @param v2 Second vertex of the triangle.
 * @param v3 Third vertex of the triangle.
 * @param texture Pointer to the sprite used as the texture.
 * @param has_transparent Whether to skip rendering transparent pixels.
 */
void renderer_draw_textured_triangle(TexturedVertex v1, TexturedVertex v2, TexturedVertex v3, const Sprite *texture, bool has_transparent);

/**
 * @brief Renders the road from the player's viewpoint.
 * @param road Pointer to the Road instance to render.
 * @param player_view Pointer to the player to calculate perspective.
 */
void renderer_draw_road(const Road *road, const Player *player_view);

/**
 * @brief Renders the player's car with optional skid effects.
 * @param player Pointer to the Player instance.
 * @param skid_input Whether skid is currently active.
 * @param player_skid_input_sign Direction of skid input (-1, 0, or 1).
 * @param cos_skid Cosine of the skid angle.
 * @param sin_skid Sine of the skid angle.
 */
void renderer_draw_player_car(const Player *player, bool skid_input, int player_skid_input_sign, float cos_skid, float sin_skid);

/**
 * @brief Renders an AI-controlled car relative to the player's view.
 * @param ai_car Pointer to the AICar instance.
 * @param player_view Pointer to the player (used for perspective).
 */
void renderer_draw_ai_car(const AICar *ai_car, const Player *player_view);

#endif //RENDERER_H
