
# makefile for msql language. 

ENV = MSYS Shell
SRC = src
INC = inc
DBG = debug

CC = gcc
CFLAGS = -g -w -O3 -I INC

APP = msql
OBJS =\
$(SRC)/main.o \
$(SRC)/tree.o \
$(SRC)/hsearch.o \
$(SRC)/parser.o \
$(SRC)/lex.o \
$(SRC)/syntax.o \
$(SRC)/stack.o \
$(SRC)/gc.o \
$(SRC)/executor.o \
$(SRC)/bin_tree.o \
$(SRC)/se.o


#run 'make' to build
all: $(APP)
$(APP):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	mv $(OBJS) $(DBG)
	mv $@ $(DBG)
	chmod 777 $(DBG)/*.*
		
$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $^ -o $@

# run 'make clean' to clean
clean:
	rm -rf $(DBG)/*.* $(SRC)/*.o $(SRC)/*.exe

#run 'make db' to debug	
db:
	$(DBG)/$(APP)

#run 'make package' to pack
package:
	tool/pack.sh

