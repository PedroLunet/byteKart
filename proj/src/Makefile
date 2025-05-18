PROG=proj

.PATH: ${.CURDIR}/controller/
.PATH: ${.CURDIR}/view/
.PATH: ${.CURDIR}/view/game/
.PATH: ${.CURDIR}/view/menus/
.PATH: ${.CURDIR}/view/utils/
.PATH: ${.CURDIR}/model/
.PATH: ${.CURDIR}/fonts/


SRCS = main.c sprite.c font.c text_renderer.c timer.c utils.c video_card.c kbc.c anim_sprite.c mouse.c ui.c game_state.c menu.c select_difficulty.c select_car.c geometry.c player.c ai_car.c game.c road.c gameover_menu.c car.c ui_utils.c
CFLAGS += -pedantic -D_LCOM_OPTIMIZED_ -O2

DPADD += ${LIBLCF}
LDADD += -Wall -llcf -lxpm -llm
.include <minix.lcom.mk>