
EXE = dev

BIN = ./bin
OBJ = ./obj
SRC = ./src
PRE = ./pre

SRCS := $(shell find $(SRC) -name *.c)
OBJS := $(SRCS:%=$(OBJ)/%.o)
RESS := $(wildcard $res/out/*.o)

CFLAGS := -Wno-void-pointer-to-int-cast
LDFLAGS := -std=c99 -lglfw -lGL -lm

CC = clang
AR = ar

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $(BIN)/$@ $(OBJS) res/out/*.o
#	$(AR) rcs $(BIN)/$@ $(OBJS)

clean:
	rm $(OBJS)

$(OBJ)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -g $(CFLAGS) -c $< -o $@

