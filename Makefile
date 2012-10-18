SOURCES=src/sanitize.c src/sanitize.h src/array.c src/array.h src/dict.c src/dict.h src/mode.c src/mode.h src/types.h src/value_checker.c src/value_checker.h

all: libsanitize.so test-app

libsanitize.so: $(SOURCES)
	gcc -g -Wall -fPIC -shared -o libsanitize.so `pkg-config --cflags --libs glib-2.0 libxml-2.0` src/sanitize.c src/array.c src/dict.c src/mode.c src/value_checker.c

test-app: libsanitize.so
	gcc -g -o test `pkg-config --cflags --libs glib-2.0 gobject-2.0 libxml-2.0` -I src t/test.c -Wl,-rpath,`pwd` -L . -lsanitize

