SRCS = main.c parser.c symbol_table.c
# Get the flags automatically from pkg-config
CFLAGS = $(shell pkg-config --cflags libxml-2.0)
LIBS = $(shell pkg-config --libs libxml-2.0)

all:
	gcc $(CFLAGS) $(SRCS) -o main $(LIBS)

clean:
	rm -f main