.PHONY : all clean

BINARIES = find-grid empty-image png-size crop-images

all : $(BINARIES)

clean :
	rm -vf $(BINARIES)

CPPFLAGS=-Wall -g -O3
LDFLAGS=-lpng12

find-grid : find-grid.c readpng.c

empty-image : empty-image.c readpng.c

png-size : png-size.c readpng.c

crop-images : crop-images.c readpng.c

