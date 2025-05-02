#include <lcom/lcf.h>

#include <stdlib.h>
#include <stdint.h>

#include "ui.h"
#include "../controller/video_card.h"

static void draw_sprite_component(UIComponent *component) {
    SpriteElementData *data = (SpriteElementData *)component->data;
    if (data->sprite) {
        sprite_draw_xpm(data->sprite, component->x, component->y);
    }
}

static void draw_container_component(UIComponent *component) {
    if (component->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)component->data;
        int x = component->x;
        int y = component->y;
        int w = data->width;
        int h = data->height;
        int bw = data->border_width;
        uint32_t bc = data->border_color;
        int br = data->border_radius;
        uint32_t bg = data->background_color;

        if (bw > 0) {
            vg_draw_rounded_rectangle(x + bw, y + bw, w - 2 * bw, h - 2 * bw, br, bg);
            vg_draw_rounded_rectangle(x, y, w, h, br, bc);
        } else {
            vg_draw_rectangle(x, y, w, h, bg);
        }

        for (int i = 0; i < data->num_children; ++i) {
            draw_ui_component(data->children[i]);
        }

    }
}


UIComponent *create_sprite_component(Sprite *sprite, int x, int y) {
    UIComponent *component = malloc(sizeof(UIComponent));
    if (!component) return NULL;
    component->type = TYPE_ELEMENT;
    component->x = x;
    component->y = y;
    component->draw = draw_sprite_component;
    component->layout = NULL;
    SpriteElementData *elem_data = malloc(sizeof(SpriteElementData));
    if (!elem_data) {
        free(component);
        return NULL;
    }
    elem_data->sprite = sprite;
    elem_data->width = sprite ? sprite->width : 0;
    elem_data->height = sprite ? sprite->height : 0;
    component->data = elem_data;
    return component;
}

UIComponent *create_container_component(int x, int y, int width, int height) {
    UIComponent *component = malloc(sizeof(UIComponent));
    if (!component) return NULL;
    component->type = TYPE_CONTAINER;
    component->x = x;
    component->y = y;
    component->draw = draw_container_component;
    component->layout = perform_container_layout;
    component->data = malloc(sizeof(ContainerData));
    if (!component->data) {
        free(component);
        return NULL;
    }
    ContainerData *data = (ContainerData *)component->data;
    data->children = NULL;
    data->num_children = 0;
    data->layout = LAYOUT_NONE;
    data->align_items = ALIGN_START;
    data->justify_content = JUSTIFY_START;
    data->padding_top = 0;
    data->padding_bottom = 0;
    data->padding_left = 0;
    data->padding_right = 0;
    data->width = width;
    data->height = height;
    if (width == 0) {
        data->is_dynamic_width = 1;
    } else {
        data->is_dynamic_width = 0;
    }
    data->gap = 0;
    data->border_width = 0;
    data->border_color = 0x000000;
    data->background_color = 0xFFFFFF;
    data->border_radius = 0;

    return component;
}

void add_child_to_container_component(UIComponent *container, UIComponent *child) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->children = realloc(data->children, (data->num_children + 1) * sizeof(UIComponent *));
        if (!data->children) {
            return;
        }
        data->children[data->num_children++] = child;
    }
}

void set_container_layout(UIComponent *container, int layout, int align, int justify) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->layout = layout;
        data->align_items = align;
        data->justify_content = justify;
    }
}

void set_container_padding(UIComponent *container, int top, int bottom, int left, int right) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->padding_top = top;
        data->padding_bottom = bottom;
        data->padding_left = left;
        data->padding_right = right;
    }
}

void set_container_gap(UIComponent *container, int gap) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->gap = gap;
    }
}

void set_container_border(UIComponent *container, int width, uint32_t color) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->border_width = width;
        data->border_color = color;
    }
}

void set_container_background_color(UIComponent *container, uint32_t color) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->background_color = color;
    }
}

void set_container_border_radius(UIComponent *container, int radius) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->border_radius = radius;
    }
}

void perform_container_layout(UIComponent *component) {
    if (component->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)component->data;
        int current_x = component->x + data->padding_left;
        int current_y = component->y + data->padding_top;

        int total_children_width = 0;
        int total_children_height = 0;
        int max_child_height = 0;
        int max_child_width = 0;

        // Calculate total width/height of children
        for (int i = 0; i < data->num_children; ++i) {
            UIComponent * currentComponent = (UIComponent *) data->children[i];
            int child_width = 0;
            int child_height = 0;

            if (currentComponent->type == TYPE_ELEMENT && ((SpriteElementData *)currentComponent->data)->sprite) {
                child_width = ((SpriteElementData *)currentComponent->data)->width;
                child_height = ((SpriteElementData *)currentComponent->data)->height;
            } else if (currentComponent->type == TYPE_CONTAINER) {
                child_width = ((ContainerData *)currentComponent->data)->width;
                child_height = ((ContainerData *)currentComponent->data)->height;
            }

            if (data->layout == LAYOUT_ROW) {
                total_children_width += child_width;
                if (i > 0 && data->gap > 0) {
                    total_children_width += data->gap;
                }
                if (child_height > max_child_height) {
                    max_child_height = child_height;
                }
            } else if (data->layout == LAYOUT_COLUMN) {
                total_children_height += child_height;
                if (i > 0 && data->gap > 0) {
                    total_children_height += data->gap;
                }
                if (child_width > max_child_width) {
                    max_child_width = child_width;
                }
            }
        }

        // Handle dynamic width/height
        if (data->is_dynamic_width) {
            if (data->layout == LAYOUT_ROW) {
                data->width = total_children_width + data->padding_left + data->padding_right;
            } else if (data->layout == LAYOUT_COLUMN) {
                data->width = max_child_width + data->padding_left + data->padding_right;
            }
        }
        if (data->is_dynamic_height) {
            if (data->layout == LAYOUT_ROW) {
                data->height = max_child_height + data->padding_top + data->padding_bottom;
            } else if (data->layout == LAYOUT_COLUMN) {
                data->height = total_children_height + data->padding_top + data->padding_bottom;
            }
        }

        // Now position the children
        current_x = component->x + data->padding_left;
        current_y = component->y + data->padding_top;

        int available_width = data->width - data->padding_left - data->padding_right;
        int available_height = data->height - data->padding_top - data->padding_bottom;

        int spacing_x = 0;
        int spacing_y = 0;

        if (data->layout == LAYOUT_ROW && data->num_children > 0) {
            if (data->justify_content == JUSTIFY_SPACE_BETWEEN && data->num_children > 1) {
                spacing_x = (available_width - total_children_width) / (data->num_children - 1 > 0 ? data->num_children - 1 : 1);
            } else if (data->justify_content == JUSTIFY_SPACE_AROUND && data->num_children > 0) {
                spacing_x = (available_width - total_children_width) / (data->num_children + 1 > 0 ? data->num_children + 1 : 1);
                current_x += spacing_x;
            } else if (data->justify_content == JUSTIFY_CENTER) {
                current_x += (available_width - total_children_width) / 2;
            }
        } else if (data->layout == LAYOUT_COLUMN && data->num_children > 0) {
            if (data->justify_content == JUSTIFY_SPACE_BETWEEN) {
                spacing_y = (available_height - total_children_height) / (data->num_children - 1 > 0 ? data->num_children - 1 : 1);
            } else if (data->justify_content == JUSTIFY_SPACE_AROUND) {
                spacing_y = (available_height - total_children_height) / (data->num_children + 1 > 0 ? data->num_children + 1 : 1);
                current_y += spacing_y;
            } else if (data->justify_content == JUSTIFY_CENTER) {
                current_y += (available_height - total_children_height) / 2;
            }
        }

        for (int i = 0; i < data->num_children; ++i) {
            UIComponent * currentComponent = (UIComponent *) data->children[i];
            int child_width = 0;
            int child_height = 0;

            if (currentComponent->type == TYPE_ELEMENT && ((SpriteElementData *)currentComponent->data)->sprite) {
                child_width = ((SpriteElementData *)currentComponent->data)->width;
                child_height = ((SpriteElementData *)currentComponent->data)->height;
            } else if (currentComponent->type == TYPE_CONTAINER) {
                child_width = ((ContainerData *)currentComponent->data)->width;
                child_height = ((ContainerData *)currentComponent->data)->height;
            }

            if (data->layout == LAYOUT_ROW) {
                data->children[i]->x = current_x;
                if (data->align_items == ALIGN_CENTER) {
                    data->children[i]->y = component->y + data->padding_top + (available_height - child_height) / 2;
                } else if (data->align_items == ALIGN_END) {
                    data->children[i]->y = component->y + data->height - data->padding_bottom - child_height;
                } else {
                    data->children[i]->y = current_y;
                }
                current_x += child_width + (data->gap > 0 ? data->gap : 0);
                if (data->justify_content == JUSTIFY_SPACE_BETWEEN) {
                    current_x += spacing_x;
                } else if (data->justify_content == JUSTIFY_SPACE_AROUND) {
                    current_x += spacing_x * 2;
                }
            } else if (data->layout == LAYOUT_COLUMN) {
                data->children[i]->y = current_y;
                if (data->align_items == ALIGN_CENTER) {
                    data->children[i]->x = component->x + data->padding_left + (available_width - child_width) / 2;
                } else if (data->align_items == ALIGN_END) {
                    data->children[i]->x = component->x + data->width - data->padding_right - child_width;
                } else {
                    data->children[i]->x = current_x;
                }
                current_y += child_height + (data->gap > 0 ? data->gap : 0);
                if (data->justify_content == JUSTIFY_SPACE_BETWEEN) {
                    current_y += spacing_y;
                } else if (data->justify_content == JUSTIFY_SPACE_AROUND) {
                    current_y += spacing_y * 2;
                }
            } else { // LAYOUT_NONE
                // Children are positioned manually
            }

            if (data->children[i]->type == TYPE_CONTAINER) {
                perform_container_layout(data->children[i]);
            }
        }
    }
}

void draw_ui_component(UIComponent *component) {
    if (component && component->draw) {
        component->draw(component);
    }
}

void destroy_ui_component(UIComponent *component) {
    if (component) {
        if (component->type == TYPE_CONTAINER) {
            ContainerData *data = (ContainerData *)component->data;
            if (data->children) {
                for (int i = 0; i < data->num_children; ++i) {
                    destroy_ui_component(data->children[i]);
                }
                free(data->children);
            }
        }
        free(component->data);
        free(component);
    }
}


