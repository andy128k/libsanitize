SOURCES=src/sanitize.c src/array.c src/dict.c src/mode.c src/value_checker.c src/quarks.c src/common.c
HEADERS=src/sanitize.h src/array.h src/dict.h src/mode.h src/value_checker.h src/quarks.h src/common.h

libsanitize.so: $(HEADERS) $(SOURCES)
	gcc -g -Wall -fPIC -shared -o libsanitize.so `pkg-config --cflags --libs libxml-2.0` $(SOURCES)

t/test-app: libsanitize.so
	gcc -g -o t/test-app `pkg-config --cflags --libs libxml-2.0` -I src t/test.c -Wl,-rpath,`pwd` -L . -lsanitize

test: t/test-app
	./t/test-app
	python t/test.py

clean:
	rm -f libsanitize.so t/test-app

