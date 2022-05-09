MAKEFLAGS += --no-builtin-rules

MKDIR := mkdir -p
RM := rm -rf
CC := gcc -c
LD := gcc

CFLAGS += -MMD -MP
CFLAGS += -W -Wall -Wextra
CFLAGS += -std=gnu99 -pedantic-errors
CFLAGS += -g3 -O0
CFLAGS += -Isrc
CFLAGS += `sdl2-config --cflags`
LDFLAGS += -g3 -O0
LIBS += `sdl2-config --libs`

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SOURCES_COMMON := $(call rwildcard, src, *.c)
SOURCES_TARGET := $(call rwildcard, target/sdl/src, *.c)
OBJECTS := $(patsubst src/%.c, obj/src/%.c.o, $(SOURCES_COMMON)) \
			$(patsubst target/sdl/src/%.c, obj/src/%.c.o, $(SOURCES_TARGET))
DIRECTORIES := $(dir $(OBJECTS))
EXECUTABLE := bin/emuwalker
DEPENDENCIES := $(patsubst obj/src/%.c.o, obj/src/%.c.d, $(OBJECTS))

all: dirs $(EXECUTABLE)

obj/%.c.o: %.c
	$(CC) $(CFLAGS) $< -o $@

obj/%.c.o: target/sdl/%.c
	$(CC) $(CFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	$(RM) bin obj

-include $(DEPENDENCIES)

dirs:
	$(MKDIR) bin $(DIRECTORIES)

.PHONY: all clean dirs
