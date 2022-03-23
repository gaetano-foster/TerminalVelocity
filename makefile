OBJECTS := $(shell find . -type f -iname "*.c")
CFLAGS  := -g -fexpensive-optimizations -flto -DUNICODE -Ofast -Wno-unknown-pragmas -std=c99
LIBS    := -L. -lm -lpthread

all:
	@gcc -o ./a.out $(OBJECTS) $(CFLAGS) $(LIBS)
