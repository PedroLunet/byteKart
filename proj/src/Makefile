PROG=proj

.PATH: ${.CURDIR}/controller/
.PATH: ${.CURDIR}/view/
.PATH: ${.CURDIR}/view/game/
.PATH: ${.CURDIR}/view/menus/
.PATH: ${.CURDIR}/view/utils/
.PATH: ${.CURDIR}/model/
.PATH: ${.CURDIR}/fonts/
.PATH: ${.CURDIR}/sprites/

SRCS = main.c sprite.c font.c text_renderer.c timer.c utils.c video_card.c kbc.c mouse.c ui.c ui_utils.c loadingUI.c game_state.c menu.c select_difficulty.c select_car.c geometry.c player.c ai_car.c renderer.c minimap.c items.c game.c road.c car.c leaderboard_menu.c obb.c physics_utils.c pause_menu.c cronometer.c finish_race_menu.c
CFLAGS += -pedantic -D_LCOM_OPTIMIZED_ -O2

DPADD += ${LIBLCF}
LDADD += -Wall -llcf -lxpm -llm
.include <minix.lcom.mk>