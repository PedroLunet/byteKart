#include <lcom/lcf.h>

#include "text_renderer.h"

int load_text(const char *text, int x, int y, uint32_t color, const Font *font, uint32_t *pixel_data, int buffer_width) {
    if (!font || !font->atlas || !font->glyphsLoaded) {
        printf("Error: Invalid font data.\n");
        return 1;
    }

    int current_x = x;
    for (int i = 0; text[i] != '\0'; i++) {
        char ch = text[i];
        GlyphData glyphData;
        if (font_get_glyph_data(font, ch, &glyphData)) {
            for (int row = 0; row < glyphData.height; row++) {
                for (int col = 0; col < glyphData.width; col++) {
                    int atlas_x = glyphData.x + col;
                    int atlas_y = glyphData.y + row;
                    int atlas_index = atlas_y * font->atlas->width + atlas_x;
                    int draw_x = current_x + glyphData.xoffset + col;
                    int draw_y = y + glyphData.yoffset + row;

                    if (atlas_index >= 0 && atlas_index < font->atlas->width * font->atlas->height) {
                        uint32_t pixel_color = font->atlas->map[atlas_index];

                        if (pixel_color != 0x00000000) {
                            int pixel_data_index = (draw_y - y) * buffer_width + (draw_x - x);
                            if (pixel_data_index >= 0 && pixel_data_index < buffer_width * font->atlas->height) {
                                pixel_data[pixel_data_index] = color;
                            }
                        }
                    }
                }
            }
            current_x += glyphData.xadvance;
        }
    }
    return 0;
}

