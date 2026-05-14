CC ?= cc
CFLAGS ?= -g -Wall -O2 -Iparser
LDFLAGS ?= -pthread

YACC ?= bison
LEX = flex
YFLAGS += -d -t -v -b y

SRC = parser/parse.c parser/scan.c src/main.c src/execute.c
OBJ = $(SRC:.c=.o)

LIBS = -lfl

all: shell

parser/parse.c: parser/parse.y
	cd parser && $(YACC) $(YFLAGS) parse.y
	mv parser/y.tab.c parser/parse.c

parser/scan.c: parser/scan.l parser/parse.c
	cd parser && $(LEX) -o scan.c scan.l

shell: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f src/*.o parser/*.o
	rm -f parser/parse.c parser/scan.c parser/y.tab.c parser/y.tab.h parser/y.output
	rm -f shell

.PHONY: all clean
