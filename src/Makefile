CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -ggdb
LDFLAGS =

TARGET = compiler
SRCS   = main.c lex.c ast.c interpreter.c
OBJS   = $(SRCS:.c=.o) arena.o stb_ds.o
DEPS   = lex.h ast.h arena.h interpreter.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

arena.o: arena.h
	$(CC) $(CFLAGS) -x c -o $@ -c $^ -DARENA_IMPLEMENTATION

stb_ds.o: stb_ds.h
	$(CC) $(CFLAGS) -x c -o $@ -c $^ -DSTB_DS_IMPLEMENTATION

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

-include $(OBJS:.o=.d)

%.d: %.c
	@$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< > $@
