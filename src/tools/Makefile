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
TOOLS = getINDI setINDI evalINDI

# supporting files comprising the INDI driver framework implementation
IDAI = ../eventloop.o

# build everything
all: tools

# build each tool
tools: $(TOOLS)

# build each INDI driver process
getINDI: $(IDAI) getINDIproperty.o
	$(CC) $(LDFLAGS) -o $@ $(IDAI) getINDIproperty.o $(LIBS)

setINDI: $(IDAI) setINDIproperty.o
	$(CC) $(LDFLAGS) -o $@ $(IDAI) setINDIproperty.o $(LIBS)

evalINDI: $(IDAI) evalINDI.o compiler.o
	$(CC) $(LDFLAGS) -o $@ $(IDAI) evalINDI.o compiler.o $(LIBS)

# clean up
clean:
	touch x.o
	rm -f *.o $(TOOLS) x.err
