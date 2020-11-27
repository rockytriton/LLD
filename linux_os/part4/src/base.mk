SHELL ?= /bin/sh
CC ?= gcc
FLAGS ?= -nostdlib -I../include
MYOS_PATH ?= /mnt/myos
DESTDIR ?= /mnt/myos

PREFIX ?= $(DESTDIR)
BINDIR ?= $(PREFIX)/sbin

OBJECTS += ../crt0_s.o

%_c.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

%_s.o: %.S
	$(CC) $(FLAGS) -c $< -o $@

install:
	cp $(TARGET) $(BINDIR)/

clean:
	rm *.o $(TARGET)
