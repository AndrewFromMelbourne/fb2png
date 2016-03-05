CFLAGS = -g -O2 -Wall  $(shell libpng-config --cflags)
LDLIBS = $(shell libpng-config --ldflags)

PROGRAMS = fb2png

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *.o

new: clean all
