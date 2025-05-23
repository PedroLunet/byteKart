#include <lcom/lcf.h>
#include "loadingUI.h"

LoadingUI *loading_ui_create(Font *font, uint16_t width, uint16_t height) {
    LoadingUI *ui = malloc(sizeof(LoadingUI));
    if (!ui) return NULL;

    const char *texts[3] = { "LOADING.  ", "LOADING.. ", "LOADING..." };
    for (int i = 0; i < 3; ++i) {
        UIComponent *container = create_container_component(0, 0, width, height);
        if (!container) { loading_ui_destroy(ui); return NULL; }
        set_container_layout(container, LAYOUT_COLUMN, ALIGN_CENTER, JUSTIFY_CENTER);
        set_container_background_color(container, 0x111111);

        UIComponent *text = create_text_component(texts[i], font, 0xFFFFFF);
        if (!text) { destroy_ui_component(container); loading_ui_destroy(ui); return NULL; }
        add_child_to_container_component(container, text);
        perform_container_layout(container);

        ui->components[i] = container;
    }
    return ui;
}

void swap_buffer_loading_ui() {
    if (swap_buffers() != 0) {
        printf("Renderer Error: Failed to swap buffers.\n");
    }
}

void loading_ui_destroy(LoadingUI *ui) {
    if (!ui) return;
    for (int i = 0; i < 3; ++i) {
        if (ui->components[i]) destroy_ui_component(ui->components[i]);
    }
    free(ui);
}
