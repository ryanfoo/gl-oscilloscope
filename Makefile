#
# Makefile
# 

# Remove MACOSX_CORE if not on OS X
CC  	= g++ -g -D__MACOSX_CORE__ -Wno-deprecated-declarations
CFLAGS	= -g -std=c99 -Wall
DEPS	= Oscillators/* Filters/* Effects/*
LIBS	= -lportaudio -lsndfile -framework OpenGL -framework GLUT -framework Cocoa

OBJS	= main.o

EXE		= main

all: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LIBS)

clean:
		rm -f *~ core $(EXE) *.o
		rm -rf main.dSYM
