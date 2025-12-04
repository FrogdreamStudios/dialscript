# Copyright © 2025 Arsenii Motorin
# Licensed under the Apache License, Version 2.0
# See: http://www.apache.org/licenses/LICENSE-2.0

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra
OBJDIR = .objects
OBJS = $(OBJDIR)/main.o $(OBJDIR)/compiler.o

dialscript: $(OBJS)
	$(CC) -o $(OBJDIR)/dialscript $(OBJS)

$(OBJDIR)/main.o: main/main.c main/main.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/compiler.o: compiler/compiler.c compiler/compiler.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: dialscript
	$(OBJDIR)/dialscript test.ds

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/dialscript

.PHONY: clean run
