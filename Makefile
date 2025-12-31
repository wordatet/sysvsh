# Modern Makefile for SVR4 sh porting
# Supports GCC (-O0/-O2) and PCC (vintage)

CC = gcc
PCC = pcc
CFLAGS_BASE = -I. -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -std=gnu89 -Wno-implicit-int -Wno-implicit-function-declaration -Wno-return-type -Wno-incompatible-pointer-types -Wno-pointer-sign
CFLAGS_O0 = $(CFLAGS_BASE) -g -O0 -Wall -Wno-parentheses -Wno-unused-variable -Wno-unused-label
CFLAGS_O2 = $(CFLAGS_BASE) -O2

# Default to O0 for development
CFLAGS = $(CFLAGS_O0)

LDFLAGS = 
LIBS = 

MAINS = sh
OBJECTS = args.o blok.o cmd.o ctype.o defs.o echo.o error.o expand.o \
	fault.o func.o hash.o hashserv.o io.o macro.o main.o msg.o \
	name.o print.o pwd.o service.o setbrk.o stak.o string.o test.o \
	word.o xec.o bltin.o jobs.o ulimit.o compat.o

SOURCES = $(OBJECTS:.o=.c)

all: $(MAINS)

sh: $(OBJECTS)
	$(CC) $(CFLAGS) -o sh $(OBJECTS) $(LDFLAGS) $(LIBS)

# Vintage build target using PCC
vintage: CC = $(PCC)
vintage: CFLAGS = -I. -D_GNU_SOURCE -O
vintage: clean $(MAINS)

# Optimized build target
opt: CFLAGS = $(CFLAGS_O2)
opt: clean $(MAINS)

# Stability Suite
test: all
	@echo "Running Paranoid Mode Stability Suite..."
	./stress.sh

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./sh -c "exit"
	@echo "Valgrind smoke test passed."

clean:
	rm -f $(OBJECTS) $(MAINS)

.c.o:
	$(CC) $(CFLAGS) -c $<

# Dependencies (simplified for now, following sh.mk logic)
$(OBJECTS): defs.h mode.h name.h stak.h brkincr.h ctype.h
