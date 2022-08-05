SRC_DIR := ./src
BUILD_DIR := ./build

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CC := gcc
CFLAGS := -g -Wall
DEPFLAGS := -MMD -MP
LDFLAGS := -lncurses

BIN := $(BUILD_DIR)/sortstudy

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

.DELETE_ON_ERROR:
.PHONY: clean run
clean:
	rm -rf $(BUILD_DIR)

run: $(BIN)
	@$(BIN) data/card.txt

-include $(DEPS)
