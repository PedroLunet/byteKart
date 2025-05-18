#include <lcom/lcf.h>

#include "ui_utils.h"

UIComponent *create_menu_option(const char *text, Font *font, int width, int height, UIComponent *parent) {
  UIComponent *container = create_container_component(0, 0, width, height);
  if (!container) return NULL;
  set_container_layout(container, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
  set_container_background_color(container, 0x111111);
  set_container_padding(container, 20, 20, 20, 20);
  set_container_border_radius(container, 10);
  set_container_border(container, 2, 0xFFFFFF);

  UIComponent *text_component = create_text_component(text, font, 0xFFFFFF);
  if (!text_component) {
      destroy_ui_component(container);
      return NULL;
  }
  add_child_to_container_component(container, text_component);
  add_child_to_container_component(parent, container);
  perform_container_layout(container);
  return container;
}

bool is_mouse_over_menu_options(GameState *base, int mouse_x, int mouse_y, UIComponent **options, size_t num_options, int *selected_option, uint32_t default_color, uint32_t hover_color) {

  *selected_option = -1;
  for (size_t i = 0; i < num_options; ++i) {
      if (options[i] && options[i]->type == TYPE_CONTAINER && options[i]->data) {
          ContainerData *data = (ContainerData *)options[i]->data;
          if (mouse_x >= options[i]->x && mouse_x < options[i]->x + data->width &&
              mouse_y >= options[i]->y && mouse_y < options[i]->y + data->height) {
              *selected_option = i;
              set_container_background_color(options[i], hover_color);
              return true;
          } else {
              set_container_background_color(options[i], default_color);
          }
      }
  }
  return false;
}

