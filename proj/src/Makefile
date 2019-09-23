PROG=proj

SRCS = proj.c timer.c video.c sprites.c bmp.c keyboard.c mouse.c RTC.c serial_port.c text.c game.c

CPPFLAGS += -pedantic -D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>
