CC=clang
STRIP=strip
CFLAGS=-Wall -O2 -ffast-math
CLIBS=-lm

EXE=hexd

all: $(EXE)

$(EXE): hexd.o
	$(CC) $(CFLAGS) $(CLIBS) -o $@ $^
	$(STRIP) $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

install:
	install -m 0755 $(EXE) /bin/$(EXE)

uninstall:
	-rm -f /bin/$(EXE)

clean:
	-rm -f hexd.o $(EXE)

.PHONY: all clean install uninstall

