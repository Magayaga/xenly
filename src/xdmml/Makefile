# XDMML Standalone Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -fPIC
AR = ar

# Platform detection
UNAME_S := $(shell uname -s)

# Platform-specific flags
ifeq ($(UNAME_S),Linux)
    CFLAGS += -DXDMML_PLATFORM_LINUX
    LDFLAGS = -lX11 -lGL -lasound
endif

ifeq ($(UNAME_S),Darwin)
    CFLAGS += -DXDMML_PLATFORM_MACOS
    LDFLAGS = -framework Cocoa -framework OpenGL -framework CoreAudio
endif

ifeq ($(UNAME_S),FreeBSD)
    CFLAGS += -DXDMML_PLATFORM_FREEBSD
    LDFLAGS = -lX11 -lGL
endif

SOURCES = xdmml_core.c xdmml_audio.c
OBJECTS = $(SOURCES:.c=.o)
LIBRARY = libxdmml.a

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^

%.o: %.c xdmml.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(LIBRARY)

install: $(LIBRARY)
	install -d /usr/local/lib
	install -m 644 $(LIBRARY) /usr/local/lib/
	install -d /usr/local/include/xdmml
	install -m 644 xdmml.h /usr/local/include/xdmml/

.PHONY: all clean install
