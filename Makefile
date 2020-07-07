# University of Crete, Greece
# HY - 340: Languages and Compilers
#

## Objects

CC = gcc
CFLAGS = -g -O0
LIBS =

OBJECTS = \
scanner.o \
parser.o \
symtable.o \
definitions.o \
target.o

AVM_OBJECTS = \
avm.o \
dispatcher.o

## Implicit rules

%.o: %.c
	@echo 'compiling $< --> $@ ...'
	@$(CC) $(CFLAGS) -c $< -o $@

%.c: %.l
	@echo 'flexing $< --> $@ ...'
	@flex --outfile=$@ $<

%.c: %.y
	@echo 'yaccing $< --> $@ ...'
	@bison -y -d -v --output=$@ $<
	
## Rules

all: parser avm
scanner.o: parser.o

avm: avm.o dispatcher.o
	@echo 'linking $@ ...'
	@$(CC) -lm -o $@ $(AVM_OBJECTS) $(LIBS)

parser: scanner.o parser.o symtable.o definitions.o target.o
	@echo 'linking $@ ...'
	@$(CC) -o $@ $(OBJECTS) $(LIBS)

clean:
	rm -f parser avm quads.i instructions.t syntax.txt symbol_table.txt *.o $(OBJECTS) $(AVM_OBJECTS) parser.h *~ parser.output
