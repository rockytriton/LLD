
LDFLAGS = -shared
TARGET = libmy.so
CSOURCES = $(shell echo *.c)
SSOURCES = $(shell echo *.S)
FLAGS = -nostdlib -I../include -fPIC

OBJECTS = $(CSOURCES:.c=_c.o)
OBJECTS += $(SSOURCES:.S=_s.o)

BINDIR = $(PREFIX)/lib

all: $(TARGET)

$(TARGET): $(OBJECTS)
	ld $(FLAGS) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

include ../base.mk
