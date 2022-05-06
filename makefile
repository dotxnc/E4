
EXE = dev

BIN = ./bin
OBJ = ./obj
SRC = ./src
PRE = ./pre

SRCS := $(shell find $(SRC) -name *.c)
OBJS := $(SRCS:%=$(OBJ)/%.o)
RESS := $(wildcard res/out/*.o)

ifeq ($(OS),Windows_NT)
	CFLAGS := -Wno-void-pointer-to-int-cast -I/home/xnc/glfw/include
	LDFLAGS := -std=c99 -L/home/xnc/glfw/build/src -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32 -lm -static -pthread
	CC = x86_64-w64-mingw32-gcc #clang
	AR = x86_64-w64-mingw32-ar #ar
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CFLAGS := -Wno-void-pointer-to-int-cast
		LDFLAGS := -std=c99 -lglfw -lGL -lm
		CC = clang
		AR = ar
	endif
endif

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $(OBJS) $(RESS) $(LDFLAGS) 
#	$(AR) rcs $(BIN)/$@ $(OBJS)

clean:
	rm $(OBJS)

$(OBJ)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -g $(CFLAGS) -c $< -o $@

