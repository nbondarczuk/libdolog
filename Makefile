CC = g++ -m32 -Wall
XERCESC = $(HOME)/work/xerces/xerces-c-3.1.1-x86-macosx-gcc-4.0
INC = -I$(XERCESC)/include
LIB = -L$(XERCESC)/lib -lxerces-c
SRC = DoLog.cpp DoLogXmlParse.cpp run.cpp
OBJ = DoLog.o DoLogXmlParse.o run.o

all: dolog

dolog:
	$(CC) -c $(SRC) $(INC) 
	$(CC) -o dolog $(OBJ) $(LIB)

clean:
	rm -f $(OBJ) dolog *~

