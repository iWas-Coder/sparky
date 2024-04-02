#
# GNU Sparky --- A 5v5 character-based libre tactical shooter
# Copyright (C) 2024 Wasym A. Alonso
#
# This file is part of Sparky.
#
# Sparky is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Sparky is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sparky.  If not, see <http://www.gnu.org/licenses/>.
#


##################
# === MACROS === #
##################
NAME = sparky

include config.mk

# Version formatting
ifeq ($(wildcard .git/), .git/)
  ifneq ($(shell which git 2>/dev/null),)
    GIT_SHA := -git+$(shell git rev-parse --short HEAD)
  endif
  DEVEXTRAVERSION := $(or $(GIT_SHA), -dev)
endif
ifeq ($(SUBLEVEL), 0)
  DIST_VERSION = $(VERSION).$(PATCHLEVEL)$(EXTRAVERSION)
else
  DIST_VERSION = $(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)
endif
FULL_VERSION = $(DIST_VERSION)$(DEVEXTRAVERSION)

# Pretty Printing Output (PPO)
PPO_MKDIR = MKDIR
PPO_CLEAN = CLEAN
PPO_CARGO = CARGO
PPO_CC    = CC
PPO_AR    = AR
PPO_LD    = LD

# Directories
SRC_DIR                 = src
HDR_DIR                 = include
BUILD_ROOT_DIR          = build
BUILD_DEBUG_DIR         = $(BUILD_ROOT_DIR)/debug
BUILD_RELEASE_DIR       = $(BUILD_ROOT_DIR)/release
LAUNCHER_BUILD_ROOT_DIR = $(BUILD_ROOT_DIR)/$(NAME)_extras
ifdef D
  LAUNCHER_BUILD_DIR = $(LAUNCHER_BUILD_ROOT_DIR)/debug
  BUILD_DIR          = $(BUILD_DEBUG_DIR)
else
  LAUNCHER_BUILD_DIR = $(LAUNCHER_BUILD_ROOT_DIR)/release
  BUILD_DIR          = $(BUILD_RELEASE_DIR)
endif
VENDOR_DIR       = vendor
RAYLIB_SRC_DIR   = $(VENDOR_DIR)/raylib/$(SRC_DIR)
RAYLIB_BUILD_DIR = $(BUILD_DIR)/raylib

# Files
RAYLIB_SRCS   := $(wildcard $(RAYLIB_SRC_DIR)/*.c)
RAYLIB_OBJS   := $(patsubst $(RAYLIB_SRC_DIR)/%.c, $(RAYLIB_BUILD_DIR)/%.o, $(RAYLIB_SRCS))
LAUNCHER_SRCS := $(wildcard $(SRC_DIR)/sk_launcher*.rs)
SRCS          := $(wildcard $(SRC_DIR)/*.c)
OBJS          := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Build flags
CC = gcc
AR = ar -rc
ifdef D
  CARGO          = cargo build --target-dir $(LAUNCHER_BUILD_ROOT_DIR)
  DEBUG_SYM_OPTS = -ggdb
else
  CARGO                = cargo build -r --target-dir $(LAUNCHER_BUILD_ROOT_DIR)
  DISABLE_ASSERTS_OPTS = -D NDEBUG
  HIDE_WARNS_OPTS      = -w
  RELEASE_OPTS         = -pipe -O3
  STRIP_OPTS           = -s
endif
define RAYLIB_CPPFLAGS
  $(DISABLE_ASSERTS_OPTS)    \
  -D PLATFORM_DESKTOP        \
  -isystem $(RAYLIB_SRC_DIR) \
  -isystem $(RAYLIB_SRC_DIR)/external/glfw/$(HDR_DIR)
endef
define CPPFLAGS
  -D SK_VERSION=$(DIST_VERSION) \
  $(DISABLE_ASSERTS_OPTS)       \
  -D _POSIX_C_SOURCE=199309L    \
  -isystem $(RAYLIB_SRC_DIR)    \
  -I $(HDR_DIR)
endef
define RAYLIB_CFLAGS
  -std=gnu99         \
  $(HIDE_WARNS_OPTS) \
  $(DEBUG_SYM_OPTS)  \
  $(RELEASE_OPTS)
endef
define CFLAGS
  -std=c17          \
  -Wall             \
  -Wextra           \
  -pedantic         \
  -Werror           \
  $(DEBUG_SYM_OPTS) \
  $(RELEASE_OPTS)
endef
define LDFLAGS
  -Wl,--build-id           \
  $(STRIP_OPTS)            \
  $(RELEASE_OPTS)          \
  -L $(BUILD_DIR)          \
  -L $(LAUNCHER_BUILD_DIR) \
  -lraylib                 \
  -lsk_launcher            \
  -lm                      \
  -static-libgcc
endef

# Build output
LAUNCHER_OUT = $(LAUNCHER_BUILD_DIR)/libsk_launcher.a
RAYLIB_OUT   = $(BUILD_DIR)/libraylib.a
ifdef D
  OUT = $(NAME)_debug
else
  OUT = $(NAME)
endif

# Build targets
TGTS = $(BUILD_DIR) $(RAYLIB_BUILD_DIR) game


###################
# === TARGETS === #
###################
.PHONY: all game clean mrproper version help

all: $(TGTS)
	@:

$(BUILD_DIR):
	@echo "  $(PPO_MKDIR)   $@"
	$(Q)mkdir -p $@

$(RAYLIB_BUILD_DIR):
	@echo "  $(PPO_MKDIR)   $@"
	$(Q)mkdir -p $@

game: $(OUT)
	@echo "INFO: $(OUT) is ready  ($(FULL_VERSION))"

$(OUT): $(RAYLIB_OUT) $(LAUNCHER_OUT) $(OBJS)
	@echo "  $(PPO_LD)      $@"
	$(Q)$(CC) $(OBJS) $(LDFLAGS) -o $@

$(RAYLIB_OUT): $(RAYLIB_OBJS)
	@echo "  $(PPO_AR)      $@"
	$(Q)$(AR) $@ $^

$(RAYLIB_BUILD_DIR)/%.o: $(RAYLIB_SRC_DIR)/%.c
	@echo "  $(PPO_CC)      $@"
	$(Q)$(CC) $(RAYLIB_CPPFLAGS) $(RAYLIB_CFLAGS) -c -MD $< -o $@

-include $(RAYLIB_BUILD_DIR)/*.d

$(LAUNCHER_OUT): $(LAUNCHER_SRCS)
	@echo "  $(PPO_CARGO)   $@"
	$(Q)$(CARGO) --lib

-include $(LAUNCHER_BUILD_DIR)/*.d

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "  $(PPO_CC)      $@"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c -MD $< -o $@

-include $(BUILD_DIR)/*.d

clean:
	@if [ -d $(BUILD_ROOT_DIR) ]; then           \
	  echo "  $(PPO_CLEAN)   $(BUILD_ROOT_DIR)"; \
	  rm -r $(BUILD_ROOT_DIR);                   \
	fi

mrproper: clean
	@if [ -e $(OUT) ]; then           \
	  echo "  $(PPO_CLEAN)   $(OUT)"; \
	  rm $(OUT);                      \
	fi

version:
	@echo $(FULL_VERSION)

help:
	@echo "Targets"
	@echo "======="
	@echo "  all      :: Build all targets marked with [*]"
	@echo "* game     :: Build the bare game"
	@echo "  clean    :: Remove the 'build' directory"
	@echo "  mrproper :: Remove and cleans everything"
	@echo "  version  :: Show the current version string"
	@echo "  help     :: Show this help and usage panel"
	@echo
	@echo "Execute 'make' or 'make all' to build all targets marked with [*]"
	@echo "For further info see the ./README.org file"
