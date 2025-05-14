#include <lcom/lcf.h>

#include "ui.h"

static void draw_sprite_component(UIComponent *component) {
    SpriteElementData *data = (SpriteElementData *)component->data;
    if (data->sprite) {
        sprite_draw_xpm(data->sprite, component->x, component->y, true);
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
        Sprite *bg_image = data->background_image;

        uint32_t hover_color = data->hover_color;
        if (data->is_hovered) {
            bg = hover_color;
        }

        if (bw > 0) {
            if (bg_image) {
                sprite_draw_xpm(bg_image, x, y, false);
            } else {
                vg_draw_rounded_rectangle(x, y, w, h, br, bc);
                vg_draw_rounded_rectangle(x + bw, y + bw, w - 2 * bw, h - 2 * bw, br, bg);
            }
        } else {
            if (bg_image) {
                sprite_draw_xpm(bg_image, x, y, false);
            } else {
                vg_draw_rounded_rectangle(x, y, w, h, br, bg);
            }
        }

        for (int i = 0; i < data->num_children; ++i) {
            draw_ui_component(data->children[i]);
        }

        data->is_hovered = false;

    }
}

static void draw_text_component(UIComponent *component) {
    TextElementData *data = (TextElementData *)component->data;
    if (data->pixel_data) {
        vg_draw_text(data->pixel_data, data->width, component->x, component->y, data->height, data->width);
    }
}

static void draw_dirty_element(UIComponent *element, int x, int y, int width, int height) {
    if (!element || element->type != TYPE_ELEMENT) return;

    SpriteElementData *data = (SpriteElementData *)element->data;

    int dirty_x1 = x;
    int dirty_y1 = y;
    int dirty_x2 = x + width;
    int dirty_y2 = y + height;

    int element_x1 = element->x;
    int element_y1 = element->y;
    int element_x2 = element->x + data->width;
    int element_y2 = element->y + data->height;

    if (element_x1 >= dirty_x2 || element_x2 <= dirty_x1 ||
        element_y1 >= dirty_y2 || element_y2 <= dirty_y1) {
        return;
    }

    sprite_draw_partial_xpm(data->sprite, x, y, width, height, false);
}

static void draw_dirty_container(UIComponent *container, int x, int y, int width, int height) {
    if (!container || container->type != TYPE_CONTAINER) return;

    ContainerData *data = (ContainerData *)container->data;

    int dirty_x1 = x;
    int dirty_y1 = y;
    int dirty_x2 = x + width;
    int dirty_y2 = y + height;

    int container_x1 = container->x;
    int container_y1 = container->y;
    int container_x2 = container->x + data->width;
    int container_y2 = container->y + data->height;

    if (container_x1 >= dirty_x2 || container_x2 <= dirty_x1 ||
        container_y1 >= dirty_y2 || container_y2 <= dirty_y1) {
        return;
    }

    int bw = data->border_width;
    uint32_t bc = data->border_color;
    int br = data->border_radius;
    uint32_t bg = data->background_color;
    Sprite *bg_image = data->background_image;

    uint32_t hover_color = data->hover_color;
    if (data->is_hovered) {
        bg = hover_color;
    }

    if (bw > 0) {
        if (bg_image) {
            sprite_draw_partial_xpm(bg_image, x, y, width, height, false);
        } else {
            vg_draw_rounded_rectangle_section(container_x1, container_y1, data->width, data->height, br, bc, dirty_x1, dirty_y1, width, height);
            vg_draw_rounded_rectangle_section(container_x1 + bw, container_y1 + bw, data->width - 2 * bw, data->height - 2 * bw, br, bg, dirty_x1, dirty_y1, width, height);
        }
    } else {
        if (bg_image) {
            sprite_draw_partial_xpm(bg_image, x, y, width, height, false);
        } else {
            vg_draw_rounded_rectangle_section(container_x1, container_y1, data->width, data->height, br, bg, dirty_x1, dirty_y1, width, height);
        }
    }
}

void draw_dirty_area(UIComponent *element, int x, int y, int width, int height) {
    int dirty_x1 = x;
    int dirty_y1 = y;
    int dirty_x2 = x + width;
    int dirty_y2 = y + height;

    int element_x1 = element->x;
    int element_y1 = element->y;

    int element_x2, element_y2;

    if (element->type == TYPE_ELEMENT) {
        SpriteElementData *data = (SpriteElementData *)element->data;
        element_x2 = element->x + data->width;
        element_y2 = element->y + data->height;
    } else if (element->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)element->data;
        element_x2 = element->x + data->width;
        element_y2 = element->y + data->height;
    } else if (element->type == TYPE_TEXT) {
        TextElementData *data = (TextElementData *)element->data;
        element_x2 = element->x + data->width;
        element_y2 = element->y + data->height;
    }

    if (element_x1 >= dirty_x2 || element_x2 <= dirty_x1 ||
        element_y1 >= dirty_y2 || element_y2 <= dirty_y1) {
        return;
    }

    if (element->type == TYPE_ELEMENT) {
        draw_dirty_element(element, x, y, width, height);
    } else if (element->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)element->data;
        draw_dirty_container(element, x, y, width, height);
        for (int i = 0; i < data->num_children; ++i) {
            UIComponent *child = data->children[i];
            draw_dirty_area(child, x, y, width, height);
        }
    } else if (element->type == TYPE_TEXT) {
        draw_text_component(element);
    }

    return;
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
    if (height == 0) {
        data->is_dynamic_height = 1;
    } else {
        data->is_dynamic_height = 0;
    }
    data->gap = 0;
    data->border_width = 0;
    data->border_color = 0x000000;
    data->background_color = 0xFFFFFF;
    data->background_image = NULL;
    data->hover_color = 0xAAAAAA;
    data->is_hovered = false;
    data->border_radius = 0;

    return component;
}

UIComponent *create_text_component(const char *text, Font *font, uint32_t color) {
    UIComponent *component = malloc(sizeof(UIComponent));
    if (!component) return NULL;
    component->type = TYPE_TEXT;
    component->x = 0;
    component->y = 0;
    component->draw = draw_text_component;
    component->layout = NULL;

    TextElementData *text_data = malloc(sizeof(TextElementData));
    if (!text_data) {
        free(component);
        return NULL;
    }
    text_data->text = strdup(text);
    text_data->color = color;
    text_data->font = font;

    text_data->width = 0;
    text_data->height = 0;
    if (text && font) {
        int temp_x = 0;
        int max_height = 0;
        for (int i = 0; text[i] != '\0'; i++) {
            GlyphData glyphData;
            if (font_get_glyph_data(font, text[i], &glyphData)) {
                temp_x += glyphData.xadvance;
                if (glyphData.height > max_height) {
                    max_height = glyphData.height;
                }
            }
        }
        text_data->width = temp_x;
        text_data->height = max_height;
    }

    text_data->pixel_data = malloc(text_data->width * text_data->height * sizeof(uint32_t));
    if (load_text(text, 0, 0, color, font, text_data->pixel_data, text_data->width) != 0) {
        free(text_data->text);
        free(text_data);
        free(component);
        return NULL;
    }

    component->data = text_data;

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

void set_container_background_image(UIComponent *container, Sprite *sprite) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->background_image = sprite;
    }
}

void set_container_hover_color(UIComponent *container, uint32_t color) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->hover_color = color;
    }
}

void is_container_hovered(UIComponent *container) {
    if (container->type == TYPE_CONTAINER) {
        ContainerData *data = (ContainerData *)container->data;
        data->is_hovered = true;
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
            } else if (currentComponent->type == TYPE_TEXT) {
                child_width = ((TextElementData *)currentComponent->data)->width;
                child_height = ((TextElementData *)currentComponent->data)->height;
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
            } else if (data->justify_content == JUSTIFY_END) {
                current_x += available_width - total_children_width;
            } else if (data->justify_content == JUSTIFY_START) {
                current_x += current_x;
            }

        } else if (data->layout == LAYOUT_COLUMN && data->num_children > 0) {
            if (data->justify_content == JUSTIFY_SPACE_BETWEEN) {
                spacing_y = (available_height - total_children_height) / (data->num_children - 1 > 0 ? data->num_children - 1 : 1);
            } else if (data->justify_content == JUSTIFY_SPACE_AROUND) {
                spacing_y = (available_height - total_children_height) / (data->num_children + 1 > 0 ? data->num_children + 1 : 1);
                current_y += spacing_y;
            } else if (data->justify_content == JUSTIFY_CENTER) {
                current_y += (available_height - total_children_height) / 2;
            } else if (data->justify_content == JUSTIFY_END) {
                current_y += available_height - total_children_height;
            } else if (data->justify_content == JUSTIFY_START) {
                current_y += current_y;
            }
        }

        for (int i = 0; i < data->num_children; ++i) {
            UIComponent * currentComponent = (UIComponent *) data->children[i];
            int child_width = 0;
            int child_height = 0;

            if (currentComponent->type == TYPE_ELEMENT) {
                child_width = ((SpriteElementData *)currentComponent->data)->width;
                child_height = ((SpriteElementData *)currentComponent->data)->height;
            } else if (currentComponent->type == TYPE_CONTAINER) {
                child_width = ((ContainerData *)currentComponent->data)->width;
                child_height = ((ContainerData *)currentComponent->data)->height;
            } else if (currentComponent->type == TYPE_TEXT) {
                child_width = ((TextElementData *)currentComponent->data)->width;
                child_height = ((TextElementData *)currentComponent->data)->height;
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
        } else if (component->type == TYPE_TEXT) {
            TextElementData *text_data = (TextElementData *)component->data;
            if (text_data->text) {
                free(text_data->text);
            }
        } else if (component->type == TYPE_ELEMENT) {
            SpriteElementData *sprite_data = (SpriteElementData *)component->data;
            sprite_destroy(sprite_data->sprite);
        }
        free(component->data);
        free(component);
    }
}


