/**
 * @file ui.h
 * @brief Defines the UI component system used for rendering and layout in menus and overlays.
 *
 * 
 */
#ifndef UI_H_
#define UI_H_

#include <stdlib.h>
#include <stdint.h>

#include "../../sprites/sprite.h"
#include "../../fonts/font.h"
#include "text_renderer.h"
#include "../../controller/video_card.h"

/**
 * @brief Forward declaration of UIComponent.
 */
typedef struct UIComponent UIComponent;

/**
 * @enum UIComponentType
 * @brief Defines the type of UI component.
 */
typedef enum {
    TYPE_ELEMENT,   /**< Sprite or visual element. */
    TYPE_CONTAINER, /**< Container for other components. */
    TYPE_TEXT       /**< Text element. */
} UIComponentType;

/**
 * @struct UIComponent
 * @brief Represents a single UI element or container.
 */
struct UIComponent {
    UIComponentType type;                      /**< Type of this component. */
    int x, y;                                  /**< Position relative to parent or screen. */
    void (*draw)(struct UIComponent *);        /**< Function pointer for drawing the component. */
    void (*layout)(struct UIComponent *);      /**< Function pointer for layout management. */
    void *data;                                /**< Pointer to type-specific data. */
};

/**
 * @struct SpriteElementData
 * @brief Data structure for a sprite-based UI component.
 */
typedef struct {
    Sprite *sprite;  /**< Sprite image to render. */
    int height;      /**< Desired height of the component. */
    int width;       /**< Desired width of the component. */
} SpriteElementData;

/**
 * @struct ContainerData
 * @brief Data structure for a container UI component.
 *
 * Supports children, padding, layout, styling, and background properties.
 */
typedef struct {
    UIComponent **children;         /**< Array of child components. */
    int num_children;               /**< Number of children. */
    enum { LAYOUT_NONE, LAYOUT_ROW, LAYOUT_COLUMN } layout;
    enum { ALIGN_START, ALIGN_CENTER, ALIGN_END } align_items;
    enum { JUSTIFY_START, JUSTIFY_CENTER, JUSTIFY_END, JUSTIFY_SPACE_BETWEEN, JUSTIFY_SPACE_AROUND } justify_content;
    int gap;
    int padding_top, padding_bottom, padding_left, padding_right;
    int width, height;
    int is_dynamic_width;
    int is_dynamic_height;
    int border_width;
    uint32_t border_color;
    uint32_t background_color;
    Sprite *background_image;
    uint32_t hover_color;
    bool is_hovered;
    int border_radius;
} ContainerData;

/**
 * @struct TextElementData
 * @brief Data structure for a text-rendering UI component.
 */
typedef struct TextElementData {
    char *text;            /**< Text content. */
    uint32_t color;        /**< Text color. */
    Font *font;            /**< Font used for rendering. */
    int width, height;     /**< Dimensions of the rendered text. */
    uint32_t *pixel_data;  /**< Cached pixel buffer. */
} TextElementData;

/**
 * @brief Draws a dirty area of a container UI component.
 *
 * @param container The container to redraw.
 * @param x X offset inside the container.
 * @param y Y offset inside the container.
 * @param width Width of the area to redraw.
 * @param height Height of the area to redraw.
 */
void draw_dirty_area(UIComponent *container, int x, int y, int width, int height);

// === Component Creation ===

UIComponent *create_sprite_component(Sprite *sprite, int x, int y);
UIComponent *create_container_component(int x, int y, int width, int height);
UIComponent *create_text_component(const char *text, Font *font, uint32_t color);

// === Container Management ===

void add_child_to_container_component(UIComponent *container, UIComponent *child);
void set_container_layout(UIComponent *container, int layout, int align, int justify);
void set_container_padding(UIComponent *container, int top, int bottom, int left, int right);
void set_container_gap(UIComponent *container, int gap);
void set_container_border(UIComponent *container, int width, uint32_t color);
void set_container_background_color(UIComponent *container, uint32_t color);
void set_container_background_image(UIComponent *container, Sprite *sprite);
void set_container_hover_color(UIComponent *container, uint32_t color);
void is_container_hovered(UIComponent *container);
void set_container_border_radius(UIComponent *container, int radius);

// === Drawing and Layout ===

/**
 * @brief Draws a UI component and all its children recursively.
 * @param component The UI component to render.
 */
void draw_ui_component(UIComponent *component);

/**
 * @brief Performs automatic layout computation for a container and its children.
 * @param container The container component.
 */
void perform_container_layout(UIComponent *container);

/**
 * @brief Recursively destroys a UI component and its children.
 * @param component The root component to destroy.
 */
void destroy_ui_component(UIComponent *component);

#endif /* UI_H_ */
