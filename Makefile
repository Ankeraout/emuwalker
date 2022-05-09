ifeq ($(TARGET),)
TARGET := sdl
endif

ifeq ($(TARGET),sdl)
include target/sdl/Makefile
else
$(error Invalid target: $(TARGET))
endif
