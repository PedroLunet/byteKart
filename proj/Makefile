PROG=proj

.PATH: ${.CURDIR}/src/

SRCS = 	main.c src/xpm.c src/timer.c src/utils.c ../lab5/graphics.c ../lab3/kbc.c ../lab3/keyboard.c

CFLAGS += -pedantic -D_LCOM_OPTIMIZED_ -O2

DPADD += ${LIBLCF}
LDADD += -Wall -llcf -lxpm -llm
.include <minix.lcom.mk>