/**
 * @file font.h
 * @brief Font rendering system for sprite-based text using glyph data from a texture atlas.
 * 
 */

#ifndef FONT_H_
#define FONT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "sprites/sprite.h"
#include "macros.h"

/**
 * @struct GlyphData
 * @brief Represents the metadata for an individual character glyph.
 *
 * Contains the position, size, and spacing details for a glyph.
 */
typedef struct GlyphData {
    int x;
    int y;
    int width;
    int height;
    int xoffset;
    int yoffset;
    int xadvance;
} GlyphData;

/**
 * @struct Font
 * @brief Represents a font loaded from an XPM image and associated metadata.
 *
 * Contains the texture atlas as a sprite and the glyph metadata for rendering text.
 */
typedef struct Font {
    Sprite *atlas;
    GlyphData glyphs[MAX_FONT_GLYPHS];
    bool glyphsLoaded;
} Font;


/**
 * @brief Loads a font from an XPM image and font metadata file.
 *
 * @param atlas_data XPM map representing the texture atlas.
 * @param fntFilename Path to the font metadata file.
 * @return Pointer to the loaded Font object (or NULL on failure).
 */
Font *font_load_xpm(xpm_map_t atlas_data, const char *fntFilename);

/**
 * @brief Frees memory and resources associated with a Font object.
 *
 * @param font Font object to destroy.
 */
void font_destroy(Font *font);

/**
 * @brief Retrieves glyph metadata for a specified character.
 *
 * @param font Pointer to the Font object.
 * @param ch Character to retrieve metadata for.
 * @param data Pointer to a GlyphData structure to populate.
 * @return true if the glyph data was found, false otherwise.
 */
bool font_get_glyph_data(const Font *font, char ch, GlyphData *data);

#endif /* FONT_H_ */

