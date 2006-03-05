# relative path to dir with libraries
LIBROOT = ../

# compiler and linker flags
CC = gcc
CLDFLAGS = -g
GCCFLAGS = -ffast-math -Wall
CFLAGS = $(CLDFLAGS) $(GCCFLAGS) -I$(LIBROOT)
LDFLAGS = $(CLDFLAGS) -L$(LIBROOT)
LIBS = -llilxml -lindicom -lm

# list each sample device
DRVS = tutorial_one tutorial_two tutorial_three tutorial_ccdpreview

# supporting files comprising the INDI driver framework implementation
IDAI = ../indidrivermain.o ../eventloop.o

# build everything
all: drivers

# build each sample driver
drivers: $(DRVS)

# build each INDI driver process
tutorial_one: $(IDAI) tutorial_one.o
	$(CC) $(LDFLAGS) -o $@ $(IDAI) tutorial_one.o $(LIBS)

tutorial_two: $(IDAI) tutorial_two.o
	$(CC) $(LDFLAGS) -o $@ $(IDAI) tutorial_two.o $(LIBS)

tutorial_three: $(IDAI) tutorial_three.o
	$(CC) $(LDFLAGS) -o $@ $(IDAI) tutorial_three.o $(LIBS) -lz

tutorial_ccdpreview: $(IDAI) tutorial_ccdpreview.o
	$(CC) $(LDFLAGS) -o $@ $(IDAI) tutorial_ccdpreview.o $(LIBS) -lz

# clean up
clean:
	touch x.o
	rm -f *.o $(DRVS) x.err
