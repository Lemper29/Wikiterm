CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11 -O2
DEBUG_FLAGS ?= -g -O0
RELEASE_FLAGS ?= -O3 -DNDEBUG

LDFLAGS ?= -lcurl

SRCDIR := ./src
BUILDDIR := temp
BINDIR := build

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

TARGET := $(BINDIR)/wiki

BUILD ?= release

ifeq ($(BUILD),debug)
CFLAGS += $(DEBUG_FLAGS)
else
CFLAGS += $(RELEASE_FLAGS)
endif

.PHONY: all debug release clean distclean dirs

all: dirs $(TARGET)

debug: BUILD := debug
debug: all

release: BUILD := release
release: all

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@printf "CC $< -> $@\n"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

dirs:
	@mkdir -p $(BUILDDIR) $(BINDIR)

-include $(DEPS)

clean:
	@printf "Cleaning build artifacts...\n"
	@rm -rf $(BUILDDIR) $(BINDIR)

distclean: clean
	@printf "No extra distribution artifacts to remove.\n"
