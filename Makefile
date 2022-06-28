CC = clang
LIBS = sdl2 freetype2 opengl openal freealut
CFLAGS = -std=c11 `pkg-config $(LIBS) --cflags` -I./glad_Core-33/include/ -Wall -Wextra
LFLAGS = -lm -ldl `pkg-config $(LIBS) --libs`

SOURCES := $(shell find . -name '*.c')
OBJS_REL = $(patsubst %.c, obj/release/%.o, $(SOURCES))
OBJS_DBG = $(patsubst %.c, obj/debug/%.o, $(SOURCES))
OBJS_REL_EX := $(patsubst %.c, obj/release/%.o, $(shell echo '$(SOURCES)' | sed 's/ /\n/g' | sed 's/.*\///'))
OBJS_DBG_EX := $(patsubst %.c, obj/debug/%.o,   $(shell echo '$(SOURCES)' | sed 's/ /\n/g' | sed 's/.*\///'))

all: dbg

rel: dirs GLSpiral_release
dbg: dirs GLSpiral_debug

GLSpiral_release: flags $(OBJS_REL)
	@echo "LD obj/release/*.o -> $@"
	@$(CC) -o $@ $(OBJS_REL_EX) $(LFLAGS) -Osize
	@strip $@
$(OBJS_REL): obj/release/%.o: %.c
	@echo "CC -Osize $<"
	@$(CC) -c $< -o obj/release/$(shell echo '$@' | sed 's/.*\///') $(CFLAGS) -O2

GLSpiral_debug: flags $(OBJS_DBG)
	@echo "LD obj/debug/*.o -> $@"
	@$(CC) -o $@ $(OBJS_DBG_EX) $(LFLAGS) -g
$(OBJS_DBG): obj/debug/%.o: %.c
	@echo "CC -g $<"
	@$(CC) -c $< -o obj/debug/$(shell echo '$@' | sed 's/.*\///') $(CFLAGS) -g

.PHONY: clean dirs flags

clean:
	rm -rf obj/ GLSpiral_*

dirs:
	mkdir -p obj obj/release obj/debug

flags:
	@printf "\n    LIBS: %s\n  CFLAGS: %s\nLIBFLAGS: %s\n\n" \
			"$(LIBS)" "$(CFLAGS)" "$(LFLAGS)"
