#
# Makefile
# 

# Remove MACOSX_CORE if not on OS X
CC  	= gcc -g -D__MACOSX_CORE__ -Wno-deprecated
CFLAGS	= -g -std=c99 -Wall
LIBS	= -lportaudio -lsndfile -framework OpenGL -framework GLUT -framework Cocoa
# Linux Libraries
# LINUX_LIBS = -lGL -lGLU -lglut -lportaudio

OBJS	= main.o

EXE		= main

all: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(LIBS)

clean:
		rm -f *~ core $(EXE) *.o
		rm -rf main.dSYM
