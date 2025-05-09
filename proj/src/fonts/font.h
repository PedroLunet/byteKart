#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>
#include <stdbool.h>
#include "sprite.h"
#include "macros.h"

typedef struct GlyphData {
    int x;
    int y;
    int width;
    int height;
    int xoffset;
    int yoffset;
    int xadvance;
} GlyphData;

typedef struct Font {
    Sprite *atlas;
    GlyphData glyphs[MAX_FONT_GLYPHS];
    bool glyphsLoaded;
} Font;

Font *font_load_xpm(xpm_map_t atlas_data, const char *fntFilename);
void font_destroy(Font *font);
bool font_get_glyph_data(const Font *font, char ch, GlyphData *data);

#endif /* FONT_H_ */

