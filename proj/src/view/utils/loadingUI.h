#ifndef LOADINGUI_H
#define LOADINGUI_H

#include "view/utils/ui.h"
#include "controller/video_card.h"
#include <stdint.h>

typedef struct {
    UIComponent *components[3];
} LoadingUI;

LoadingUI *loading_ui_create(Font *font, uint16_t width, uint16_t height);
void swap_buffer_loading_ui();
void loading_ui_destroy(LoadingUI *ui);

#endif //LOADINGUI_H
