#include <lcom/lcf.h>

#include "text_renderer.h"

extern uint8_t scancode;

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

char scancode_to_char(uint8_t scancode) {
    switch (scancode) {
        case 0x1E: return 'A';  
        case 0x30: return 'B';  
        case 0x2E: return 'C';  
        case 0x20: return 'D';  
        case 0x12: return 'E';  
        case 0x21: return 'F';  
        case 0x22: return 'G';  
        case 0x23: return 'H';  
        case 0x17: return 'I';  
        case 0x24: return 'J';  
        case 0x25: return 'K';  
        case 0x26: return 'L';  
        case 0x32: return 'M';  
        case 0x31: return 'N';  
        case 0x18: return 'O';  
        case 0x19: return 'P';  
        case 0x10: return 'Q';  
        case 0x13: return 'R';  
        case 0x1F: return 'S';  
        case 0x14: return 'T';  
        case 0x16: return 'U';  
        case 0x2F: return 'V';  
        case 0x11: return 'W';  
        case 0x2D: return 'X';  
        case 0x15: return 'Y';  
        case 0x2C: return 'Z';  
        
        case 0x02: return '1';  
        case 0x03: return '2';  
        case 0x04: return '3';  
        case 0x05: return '4';  
        case 0x06: return '5';  
        case 0x07: return '6';  
        case 0x08: return '7';  
        case 0x09: return '8';  
        case 0x0A: return '9';  
        case 0x0B: return '0';  
        
        default: return '\0';
    }
}
