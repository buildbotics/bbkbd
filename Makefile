NAME = bbkbd

PKG_CONFIG = pkg-config
PKGS = fontconfig freetype2 x11 xtst xft xinerama xcursor

CDEFS = -D_DEFAULT_SOURCE -DXINERAMA
CFLAGS += -I. `$(PKG_CONFIG) --cflags $(PKGS)` $(CDEFS)
CFLAGS += -MD -MP -MT $@ -MF build/dep/$(@F).d
CFLAGS += -Wall -Werror -g
LDFLAGS += `$(PKG_CONFIG) --libs $(PKGS)`

SRC = $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))

all: $(NAME)

build/%.o: src/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

tidy:
	rm -f *~ \#*

clean: tidy
	rm -rf $(NAME) build

.PHONY: all clean tidy

# Dependencies
-include $(shell mkdir -p build/dep) $(wildcard build/dep/*)
