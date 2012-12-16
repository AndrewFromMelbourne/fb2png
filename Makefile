CFLAGS=-g -O2 -Wall 
LDFLAGS=-lpng

PROGRAMS=$(basename $(wildcard *.c))

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *.o

new: clean all
