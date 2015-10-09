.SUFFIXES : .c .o

CC = gcc

CFALGS = -g -Wall -ansi 
 all: hw

OBJS = ./src/main.o
SRCS = ./src/$(OBJS: .o=.c)
BIN  = ./bin/
LIB  =  /path/to/libs
LIBNAME = -lm

hw:	$(OBJS)
	$(CC) -o $(BIN)homework $(OBJS) -L$(LIB) $(LIBNAME) -pthread 	

clean:
	rm -rf $(OBJS)

allclean:
	rm -rf $(OBJS) $(BIN)*
