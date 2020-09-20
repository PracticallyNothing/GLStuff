CC    = clang
FLAGS = -std=c99 `pkg-config sdl2 freetype2 opengl openal --cflags` -I./glad_Core-33/include/ -Wall
LIBS  = -lm -ldl `pkg-config sdl2 freetype2 opengl openal --libs`

SOURCES := $(shell find . -name '*.c')
OBJS_REL = $(patsubst %.c, obj/release/%.o, $(SOURCES))
OBJS_DBG = $(patsubst %.c, obj/debug/%.o, $(SOURCES))
OBJS_REL_EX := $(patsubst %.c, obj/release/%.o, $(shell echo '$(SOURCES)' | sed 's/ /\n/g' | sed 's/.*\///'))
OBJS_DBG_EX := $(patsubst %.c, obj/debug/%.o,   $(shell echo '$(SOURCES)' | sed 's/ /\n/g' | sed 's/.*\///'))

all: dbg

rel: dirs GLSpiral_release
dbg: dirs GLSpiral_debug

GLSpiral_release: $(OBJS_REL)
	$(CC) -o $@ $(OBJS_REL_EX) $(LIBS) $(FLAGS) -O2
$(OBJS_REL): obj/release/%.o: %.c
	$(CC) -c $< -o obj/release/$(shell echo '$@' | sed 's/.*\///') $(FLAGS) -O2

GLSpiral_debug: $(OBJS_DBG)
	$(CC) -o $@ $(OBJS_DBG_EX) $(LIBS) $(FLAGS) -g #-fsanitize=leak
$(OBJS_DBG): obj/debug/%.o: %.c
	$(CC) -c $< -o obj/debug/$(shell echo '$@' | sed 's/.*\///') $(FLAGS) -g


.PHONY: clean dirs

clean:
	rm -rf obj/ GLSpiral_*

dirs:
	mkdir -p obj obj/release obj/debug
