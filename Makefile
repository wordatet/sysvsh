# Modern Makefile for SVR4 sh porting
# Supports GCC (-O0/-O2) and PCC (vintage)

CC = gcc
PCC = pcc
CFLAGS = -I. -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -std=gnu89 -O2 \
	-Wno-implicit-int -Wno-implicit-function-declaration -Wno-return-type \
	-Wno-incompatible-pointer-types -Wno-pointer-sign -Wno-unused-result

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
