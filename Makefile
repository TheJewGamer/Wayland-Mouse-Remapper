CC      := gcc
CFLAGS  := -g -Wall -Wextra $(shell pkg-config --cflags dbus-1)
LDLIBS  := -lpthread $(shell pkg-config --libs dbus-1)

# source files location
SRCS   := $(wildcard src/*.c)

# output location
BINDIR := bin
TARGET := $(BINDIR)/wayland-mouse-remapper

# put object files in bin when created
OBJS   := $(patsubst src/%.c,$(BINDIR)/%.o,$(SRCS))

# tell make these names are not real files
.PHONY: clean run

# produce final file/executable
$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(OBJS) $(LDLIBS) -o $@

# create BIN folder if it does not exists already
$(BINDIR):
	mkdir -p $(BINDIR)

# compile all found .c files to .o files
$(BINDIR)/%.o: src/%.c | $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# make run command
run: $(TARGET)
	DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus $(TARGET)

# make clean command
clean:
	rm -f $(OBJS) $(TARGET)