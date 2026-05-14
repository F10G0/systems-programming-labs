CC ?= cc
CFLAGS ?= -g -Wall -O2
CFLAGS += -Iparser
LDFLAGS ?=

YACC ?= bison
LEX ?= flex
YFLAGS += -d -t -v -b y

SRC = \
	src/main.c \
	src/execute.c \
	parser/parse.c \
	parser/scan.c

OBJ = $(SRC:.c=.o)

LIBS = -lfl

all: shell

shell: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

parser/parse.c parser/parse.h: parser/parse.y
	$(YACC) $(YFLAGS) -o parser/parse.c parser/parse.y

parser/scan.c: parser/scan.l parser/parse.h
	$(LEX) -o parser/scan.c parser/scan.l

clean:
	rm -f $(OBJ)
	rm -f parser/parse.c parser/parse.h parser/scan.c parser/y.output
	rm -f shell

.PHONY: all clean
