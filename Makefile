CC      = gcc
FLAGS   = -std=c99
INCLUDE = -I./glad_Core-33/include/
LIBS    = -lm -ldl -lSDL2 -lGL
SOURCES := $(shell find . -name '*.c')
OBJECTS = $(patsubst %.c, obj/%.o, $(SOURCES))
OBJS_EX := $(patsubst %.c, obj/%.o, $(shell echo '$(SOURCES)' | sed 's/ /\n/g' | sed 's/.*\///'))

all: dirs GLSpiral

GLSpiral: $(OBJECTS)
	$(CC) -o $@ $(OBJS_EX) $(LIBS) $(FLAGS)
$(OBJECTS): obj/%.o: %.c
	$(CC) -c $< -o obj/$(shell echo '$@' | sed 's/.*\///') $(INCLUDE)

.PHONY: clean
.PHONY: dirs
clean:
	rm -f GLSpiral $(OBJS_EX)
dirs:
	mkdir -p obj
