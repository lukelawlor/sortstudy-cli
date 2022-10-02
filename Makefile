SRC_DIR := ./src
BUILD_DIR := ./build

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CC := gcc
CFLAGS := -Wall -Wextra -Werror -O2 -Wimplicit-fallthrough=0
DEPFLAGS := -MMD -MP
LDFLAGS := $(shell ncursesw5-config --cflags --libs)

BINNAME := sortstudycli
BINPATH := $(BUILD_DIR)/$(BINNAME)

all: $(BINPATH)

$(BINPATH): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

.DELETE_ON_ERROR:
.PHONY: clean install uninstall
clean:
	rm -rf $(BUILD_DIR)

install:
	cp -f $(BINPATH) /usr/local/bin/$(BINNAME)
	cp -f doc/sortstudycli.1 /usr/local/share/man/man1/sortstudycli.1
	chmod 644 /usr/local/share/man/man1/sortstudycli.1

uninstall:
	rm -f /usr/local/bin/$(BINNAME)\
		/usr/local/share/man/man1/sortstudycli.1

-include $(DEPS)
