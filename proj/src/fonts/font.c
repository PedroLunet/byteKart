#include <lcom/lcf.h>
#include "font.h"
#include "sprite.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_token(char *line, int index, char delimiter) {
    char *token = NULL;
    char *current = line;
    int count = 0;

    while (current != NULL && count <= index) {
        char *next = strchr(current, delimiter);
        if (count == index) {
            if (next != NULL) {
                size_t len = next - current;
                token = malloc(len + 1);
                strncpy(token, current, len);
                token[len] = '\0';
            } else {
                token = strdup(current);
            }
            break;
        }
        current = next != NULL ? next + 1 : NULL;
        count++;
    }
    return token;
}

Font *font_load_xpm(xpm_map_t atlas_data, const char *fntFilename) {
    Font *font = malloc(sizeof(Font));
    if (!font) {
        return NULL;
    }
    font->glyphsLoaded = false;
    memset(font->glyphs, 0, sizeof(font->glyphs));

    // Load the font atlas (PNG image)
    font->atlas = sprite_create_xpm(atlas_data, 0, 0, 0, 0);
    if (!font->atlas) {
        free(font);
        return NULL;
    }

    FILE *fntFile = fopen(fntFilename, "r");
    if (!fntFile) {
        sprite_destroy(font->atlas);
        free(font);
        return NULL;
    }

    char line[512];
    while (fgets(line, sizeof(line), fntFile)) {
        if (strncmp(line, "char id=", 8) == 0) {
            int id, x, y, width, height, xoffset, yoffset, xadvance;
            if (sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                       &id, &x, &y, &width, &height, &xoffset, &yoffset, &xadvance) == 8) {
                if (id >= 0 && id < MAX_FONT_GLYPHS) {
                    font->glyphs[id].x = x;
                    font->glyphs[id].y = y;
                    font->glyphs[id].width = width;
                    font->glyphs[id].height = height;
                    font->glyphs[id].xoffset = xoffset;
                    font->glyphs[id].yoffset = yoffset;
                    font->glyphs[id].xadvance = xadvance;
                }
           } else {
               printf("Warning: Invalid format in .fnt file: %s", line);
           }
        }
    }

    fclose(fntFile);
    font->glyphsLoaded = true;
    return font;
}

void font_destroy(Font *font) {
    if (font) {
        sprite_destroy(font->atlas);
        free(font);
    }
}

bool font_get_glyph_data(const Font *font, char ch, GlyphData *data) {
    if (!font || !font->glyphsLoaded || ch < 0 || (unsigned int)ch >= MAX_FONT_GLYPHS) {
        return false;
    }
    *data = font->glyphs[(int)ch];
    return true;
}

