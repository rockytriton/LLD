
LDFLAGS = -shared
TARGET = libmydrm.so
CSOURCES = $(shell echo *.c)
FLAGS = -nostdlib -I../include -fPIC

OBJECTS = $(CSOURCES:.c=_c.o)

BINDIR = $(PREFIX)/lib

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ld $(FLAGS) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

include ../base.mk
