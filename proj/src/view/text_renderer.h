#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <stdint.h>
#include <stdbool.h>
#include "../fonts/font.h"
#include "../sprite.h"

int load_text(const char *text, int x, int y, uint32_t color, const Font *font, uint32_t *pixel_data, int buffer_width);

#endif //TEXT_RENDERER_H
