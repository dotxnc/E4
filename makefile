
EXE = libe4

BIN = ./bin
OBJ = ./obj
SRC = ./src
PRE = ./pre

SRCS := $(shell find $(SRC) -name *.c)
OBJS := $(SRCS:%=$(OBJ)/%.o)
RESS := $(wildcard res/out/*.o)

ifeq ($(OS),Windows_NT)
	CFLAGS := -Wno-void-pointer-to-int-cast -I/home/xnc/glfw/include
	CC = x86_64-w64-mingw32-gcc
	AR = x86_64-w64-mingw32-ar
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CFLAGS := -Wno-void-pointer-to-int-cast
		CC = clang
		AR = ar
	endif
endif

BUILD ?= debug

ifeq ($(BUILD),release)
	CFLAGS += -Os
endif
ifeq ($(BUILD),debug)
	CFLAGS += -g
endif

all: $(EXE)

$(EXE): $(OBJS)
	$(AR) rcs $(BIN)/$@.a $(OBJS) $(RESS)

clean:
	rm $(OBJS)

$(OBJ)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

