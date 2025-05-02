#ifndef UI_H_
#define UI_H_

#include "../sprite.h"
#include <stdint.h>

// Forward declaration
typedef struct UIComponent UIComponent;

// Enum for the type of UI component
typedef enum {
    TYPE_ELEMENT, // like a sprite
    TYPE_CONTAINER // that can contain other components
} UIComponentType;

// Base structure for all UI components
struct UIComponent {
    UIComponentType type;
    int x;
    int y;
    // Common properties can be added here (e.g., width, height, visibility)
    void (*draw)(struct UIComponent *);
    void (*layout)(struct UIComponent *);
    void *data; // Pointer to type-specific data (children, sprite, etc.)
};

// Structure for sprite elements
typedef struct {
    Sprite *sprite;
    int height;
    int width;
} SpriteElementData;

// Structure for container data
typedef struct {
    UIComponent **children;
    int num_children;
    enum { LAYOUT_NONE, LAYOUT_ROW, LAYOUT_COLUMN } layout;
    enum { ALIGN_START, ALIGN_CENTER, ALIGN_END } align_items;
    enum { JUSTIFY_START, JUSTIFY_CENTER, JUSTIFY_END, JUSTIFY_SPACE_BETWEEN, JUSTIFY_SPACE_AROUND } justify_content;
    int gap;
    int padding_top;
    int padding_bottom;
    int padding_left;
    int padding_right;
    int width;
    int height;
    int is_dynamic_width;
    int is_dynamic_height;
    int border_width;
    uint32_t border_color;
    uint32_t background_color;
    int border_radius;
} ContainerData;

// Creation functions
UIComponent *create_sprite_component(Sprite *sprite, int x, int y);
UIComponent *create_container_component(int x, int y, int width, int height);

// Management functions for containers
void add_child_to_container_component(UIComponent *container, UIComponent *child);
void set_container_layout(UIComponent *container, int layout, int align, int justify);
void set_container_padding(UIComponent *container, int top, int bottom, int left, int right);
void set_container_gap(UIComponent *container, int gap);
void set_container_border(UIComponent *container, int width, uint32_t color);
void set_container_background_color(UIComponent *container, uint32_t color);
void set_container_border_radius(UIComponent *container, int radius);

// Drawing function for all components
void draw_ui_component(UIComponent *component);

// Layout function for containers
void perform_container_layout(UIComponent *container);

// Helper function to destroy a UI component and its children (recursively)
void destroy_ui_component(UIComponent *component);

#endif /* UI_H_ */
