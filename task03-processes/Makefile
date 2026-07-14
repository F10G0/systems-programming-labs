CC ?= cc
CFLAGS ?= -g -Wall -O2
LDFLAGS ?= -pthread
LDLIBS ?= -lfl

YACC := bison
LEX := flex
YFLAGS ?= -d -t -v -b y

BUILD_DIR ?= build
GEN_DIR := $(BUILD_DIR)/generated
OBJ_DIR := $(BUILD_DIR)/obj
TARGET := $(BUILD_DIR)/shell

PARSE_C := $(GEN_DIR)/y.tab.c
PARSE_H := $(GEN_DIR)/y.tab.h
SCAN_C := $(GEN_DIR)/scan.c
OBJ := \
	$(OBJ_DIR)/parse.o \
	$(OBJ_DIR)/scan.o \
	$(OBJ_DIR)/main.o \
	$(OBJ_DIR)/execute.o

.PHONY: all clean

all: $(TARGET)

$(PARSE_C) $(PARSE_H) &: parser/parse.y | $(GEN_DIR)
	cd $(GEN_DIR) && $(YACC) $(YFLAGS) ../../parser/parse.y

$(SCAN_C): parser/scan.l $(PARSE_H) | $(GEN_DIR)
	$(LEX) -o $@ $<

$(OBJ_DIR)/parse.o: $(PARSE_C) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -Iparser -I$(GEN_DIR) -c -o $@ $<

$(OBJ_DIR)/scan.o: $(SCAN_C) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -Iparser -I$(GEN_DIR) -c -o $@ $<

$(OBJ_DIR)/main.o: src/main.c parser/parse.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -Iparser -I$(GEN_DIR) -c -o $@ $<

$(OBJ_DIR)/execute.o: src/execute.c parser/parse.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -Iparser -I$(GEN_DIR) -c -o $@ $<

$(TARGET): $(OBJ) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(GEN_DIR) $(OBJ_DIR) $(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)
