
EXE = dev

BIN = ./bin
OBJ = ./obj
SRC = ./src
PRE = ./pre

SRCS := $(shell find $(SRC) -name *.c)
OBJS := $(SRCS:%=$(OBJ)/%.o)
RESS := $(wildcard res/out/*.o)

E4_PATH := /home/xnc/Projects/E4

ifeq ($(OS),Windows_NT)
	CFLAGS := -Wno-pointer-to-int-cast -I/home/xnc/glfw/include -I$(E4_PATH)/src
	LDFLAGS := -std=c99 -L$(E4_PATH)/bin -L/home/xnc/glfw/build/src -le4 -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32 -lm -static -pthread
	CC = x86_64-w64-mingw32-gcc
	AR = x86_64-w64-mingw32-ar
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CFLAGS := -Wno-void-pointer-to-int-cast -I$(E4_PATH)/src
		LDFLAGS := -std=c99 -L$(E4_PATH)/bin -le4 -lglfw -lGL -lm
		CC = clang
		AR = ar
	endif
endif

all: $(EXE)

$(EXE): $(OBJS)
	mkdir -p bin/
	$(CC) $(CFLAGS) -o $(BIN)/$@ $(OBJS) $(RESS) $(LDFLAGS)

clean:
	rm $(OBJS)

$(OBJ)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -g $(CFLAGS) -c $< -o $@

