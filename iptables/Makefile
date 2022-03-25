CFLAGS = -O2 -Wall

all: libipt_TCPWIN.so

lib%.so: lib%.o
	gcc -shared -fPIC -o $@ $^;

lib%.o: lib%.c
	gcc ${CFLAGS} -D_INIT=lib$*_init -fPIC -c -o $@ $<;

install: libipt_TCPWIN.so
	install -D $< $(DESTDIR)$(shell pkg-config --variable xtlibdir xtables)/$<

clean:
	rm -f *.{o,so}
