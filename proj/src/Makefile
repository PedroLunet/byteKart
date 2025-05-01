PROG=proj

.PATH: ${.CURDIR}/src/
.PATH: ${.CURDIR}/src/controller/

SRCS = 	main.c sprite.c timer.c utils.c video_card.c kbc.c menu.c anim_sprite.c mouse.c
CFLAGS += -pedantic -D_LCOM_OPTIMIZED_ -O2

DPADD += ${LIBLCF}
LDADD += -Wall -llcf -lxpm -llm
.include <minix.lcom.mk>