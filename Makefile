CC := gcc
SRC_DIR := src
OBJ_DIR := build/objs
BIN_DIR := build
EXECUTABLE := $(BIN_DIR)/bupmusic

SRCS := $(shell find $(SRC_DIR) -type f -name "*.c")
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
CPP_SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp")
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_SRCS))
CFLAGS = -Wall -g -I src -fdiagnostics-color=always
LDFLAGS :=
LIBS =

ifeq ($(OS),Windows_NT)
	CFLAGS += -DWINDOWS $(shell pkg-config --static --cflags sdl3)
	LDFLAGS += $(shell pkg-config --static --libs sdl3)
else
	CFLAGS += -DWINDOWS $(shell pkg-config --cflags sdl3)
	LDFLAGS += $(shell pkg-config --libs sdl3) -lm -lstdc++
endif

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS) $(CPP_OBJS)
	@printf "\033[1m\033[32mLinking \033[36m$(OBJ_DIR) \033[32m-> \033[33m$(EXECUTABLE)\033[0m\n"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@printf "\033[1m\033[32mCompiling \033[36m$< \033[32m-> \033[33m$@\033[0m\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@printf "\033[1m\033[32mCompiling \033[36m$< \033[32m-> \033[33m$@\033[0m\n"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@printf "\033[1m\033[32mDeleting \033[36m$(BIN_DIR) \033[32m-> \033[31mX\033[0m\n"
	@rm -rf $(BIN_DIR)

-include $(OBJS:.o=.d)

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< -o $@

$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< -o $@

-include $(OBJS:.o=.d)
