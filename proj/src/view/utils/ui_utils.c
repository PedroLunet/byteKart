#include <lcom/lcf.h>

#include "ui_utils.h"

extern vbe_mode_info_t vbe_mode_info;

// Função genérica para criar botões em menu
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

  if (!container) {
    destroy_ui_component(parent);
    return NULL;
  }
  return container;
}

// Função genérica para verificar se o rato está dar hover
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

// Função genérica para criar back button
UIComponent *create_back_button(Font *font, UIComponent *parent) {
    UIComponent *backButton = create_container_component(30, 30, 40, 40);
    if (!backButton) return NULL;

    set_container_layout(backButton, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
    set_container_background_color(backButton, 0x00BB00);
    set_container_hover_color(backButton, 0x00DD00);
    set_container_border_radius(backButton, 15);
    set_container_border(backButton, 2, 0x00DD00);

    UIComponent *backText = create_text_component("<", font, 0xFFFFFF);
    if (!backText) {
        destroy_ui_component(backButton);
        return NULL;
    }
    add_child_to_container_component(backButton, backText);
    perform_container_layout(backButton);

    if (!backButton) {
        destroy_ui_component(parent);
        return NULL;
    }

    return backButton;
}

// Função genérica para criar títulos
UIComponent *create_title_text(const char *text, Font *font, uint32_t color, UIComponent *parent) {
    UIComponent *titleText = create_text_component(text, font, color);
    if (!titleText) {
        destroy_ui_component(parent);
        return NULL;
    }
    add_child_to_container_component(parent, titleText);
    return titleText;
}

// Função genérica para criar main containers
UIComponent *create_main_container(Sprite *background, int gap, int top_pad, int bot_pad, int left_pad, int right_pad) {

    UIComponent* mainContainer = create_container_component(0, 0, vbe_mode_info.XResolution, vbe_mode_info.YResolution);
    if (!mainContainer) {
        return NULL;
    }
    set_container_layout(mainContainer, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);

    if (background != NULL) {
        set_container_background_image(mainContainer, background);
    } else {
        set_container_background_color(mainContainer, 0x111111);
    }

    set_container_padding(mainContainer, top_pad, bot_pad, left_pad, right_pad);
    set_container_gap(mainContainer, gap);


    return mainContainer;
}

