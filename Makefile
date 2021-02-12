CFLAGS = -g -O2 -Wall `pkg-config --cflags libpng`
LDLIBS = `pkg-config --libs libpng`

PROGRAMS = fb2png

all: $(PROGRAMS)

clean:
	rm -f $(PROGRAMS) *.o

new: clean all
