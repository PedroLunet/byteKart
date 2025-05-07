#include <lcom/lcf.h>
#include "road.h"

  void draw_road_background() {
    int screen_width = 800;
    int screen_height = 600;

    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            if (x < 150 || x > 650) {
                vg_draw_pixel(x, y, 0x007700);
            }
            else if (x >= 390 && x <= 410 && (y / 20) % 2 == 0) {
                vg_draw_pixel(x, y, 0xFFFFFF); 
            }
            else {
                vg_draw_pixel(x, y, 0x444444); 
            }
        }
    }
}


