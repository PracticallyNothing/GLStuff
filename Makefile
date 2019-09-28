CC      = gcc
FLAGS   = -std=c99 
INCLUDE = -I./glad_Core-33/include/
LIBS    = -lm -ldl -lSDL2 -lGL
SOURCES := $(shell find . -name '*.c')
OBJS_REL = $(patsubst %.c, obj/release/%.o, $(SOURCES))
OBJS_DBG = $(patsubst %.c, obj/debug/%.o, $(SOURCES))
OBJS_REL_EX := $(patsubst %.c, obj/release/%.o, $(shell echo '$(SOURCES)' | sed 's/ /\n/g' | sed 's/.*\///'))
OBJS_DBG_EX := $(patsubst %.c, obj/debug/%.o, $(shell echo '$(SOURCES)' | sed 's/ /\n/g' | sed 's/.*\///'))


all: dbg 

rel: release
dbg: debug

release: dirs GLSpiral_release
debug: dirs GLSpiral_debug

GLSpiral: GLSpiral_release

GLSpiral_release: $(OBJS_REL)
	$(CC) -o $@ $(OBJS_REL_EX) $(LIBS) $(FLAGS) -O2
$(OBJS_REL): obj/release/%.o: %.c
	$(CC) -c $< -o obj/release/$(shell echo '$@' | sed 's/.*\///') $(INCLUDE)

GLSpiral_debug: $(OBJS_DBG)
	$(CC) -o $@ $(OBJS_DBG_EX) $(LIBS) $(FLAGS) -g
$(OBJS_DBG): obj/debug/%.o: %.c
	$(CC) -c $< -o obj/debug/$(shell echo '$@' | sed 's/.*\///') $(INCLUDE) -g

.PHONY: clean dirs
again: clean all
clean:
	rm -rf obj/ GLSpiral_*
dirs:
	mkdir -p obj obj/release obj/debug
