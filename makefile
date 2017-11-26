SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
SDL2_CFLAGS ?= $(shell pkg-config --cflags SDL2_image)
SDL2_LDLIBS ?= $(shell pkg-config --libs SDL2_image)
CFLAGS ?= --std=c11 -g -Wall -Wextra -O3 $(SDL2_CFLAGS)
LDLIBS ?= $(SDL2_LDLIBS) -lm

c3do: $(OBJECTS)
	$(CC) $(CFLAGS) -o c3do $(OBJECTS) $(LDLIBS)
