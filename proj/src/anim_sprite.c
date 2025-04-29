#include <lcom/lcf.h>
#include <anim_sprite.h>

AnimSprite *create_animSprite(uint8_t no_pic, xpm_map_t pic1[], ...) {

    AnimSprite *asp = malloc(sizeof(AnimSprite));
    if (asp == NULL) return NULL;

    asp->sp = create_sprite_xpm(pic1, 0, 0, 0, 0);
    if (asp->sp == NULL) {
        free(asp);
        return NULL;
    }

    asp->map = malloc(no_pic * sizeof(uint32_t *));
    if (asp->map == NULL) {
        destroy_sprite(asp->sp);
        free(asp);
        return NULL;
    }

    asp->map[0] = (uint32_t *) asp->sp->map;

    va_list ap;
    va_start(ap, pic1);
    for (int i = 1; i < no_pic; i++) {
        xpm_map_t tmp = va_arg(ap, xpm_map_t);
        xpm_image_t img;
        asp->map[i] = (uint32_t *) xpm_load(tmp, XPM_8_8_8_8, &img);
        if (asp->map[i] == NULL || img.width != asp->sp->width || img.height != asp->sp->height) {
            for (int j = 0; j < i; j++)
                free(asp->map[j]);
            free(asp->map);
            destroy_sprite(asp->sp);
            free(asp);
            va_end(ap);
            return NULL;
        }
    }
    va_end(ap);

    asp->aspeed = 0;
    asp->cur_aspeed = 0;
    asp->num_fig = no_pic;
    asp->cur_fig = 0;

    return asp;
}

int animate_animSprite(AnimSprite *sp,) {

    if (sp == NULL || sp->num_fig <= 1) return 1;

    if (sp->cur_aspeed == 0) {

        sp->cur_fig = (sp->cur_fig + 1) % sp->num_fig;
        sp->sp->map = sp->map[sp->cur_fig];
        sp->cur_aspeed = sp->aspeed;

    } else {
        sp->cur_aspeed--;
    }

    return 0;

}

void destroy_animSprite(AnimSprite *sp) {
    if (sp == NULL) return;

    for (int i = 0; i < sp->num_fig; i++)
        free(sp->map[i]);
    free(sp->map);
    destroy_sprite(sp->sp);
    free(sp);
}
