CC =		cc
CFLAGS =	-Wall -g `pkg-config --cflags kcgi-html sqlbox`
LDFLAGS =	-static  `pkg-config --libs   kcgi-html sqlbox` -lpthread -lm

ETAGS =		etags
SQLITE3 =	sqlite3

SQLPORTS =	/usr/local/share/sqlports

.PHONY: all clean install

all: webpkg webpkg.sqlite TAGS

webpkg: webpkg.o views.o
	${CC} webpkg.o views.o -o $@ ${LDFLAGS}

webpkg.sqlite: ${SQSLPORTS} gen.sql
	cp ${SQLPORTS} webpkg.sqlite
	${SQLITE3} webpkg.sqlite < gen.sql

TAGS: webpkg.c views.c
	@-${ETAGS} webpkg.c || true

clean:
	rm -f webpkg *.o *.core webpkg.sqlite

install: webpkg webpkg.sqlite webpkg.css
	doas install -m0555 webpkg /var/www/cgi-bin/webpkg.cgi
	doas install -m0444 webpkg.sqlite /var/www/
	doas install -m0444 webpkg.css /var/www/htdocs
