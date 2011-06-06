# mwm - manual window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = mwm.c
OBJ = ${SRC:.c=.o}

all: options mwm

options:
	@echo mwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo ${CC} -c ${CFLAGS} $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

mwm: ${OBJ}
	@echo ${CC} -o $@ ${OBJ} ${LDFLAGS}
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f mwm ${OBJ} mwm-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p mwm-${VERSION}
	@cp -R LICENSE Makefile README config.mk \
		mwm.1 ${SRC} mwm-${VERSION}
	@tar -cf mwm-${VERSION}.tar mwm-${VERSION}
	@gzip mwm-${VERSION}.tar
	@rm -rf mwm-${VERSION}

.PHONY: all options clean dist
