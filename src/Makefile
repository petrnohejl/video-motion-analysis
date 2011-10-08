
CC=gcc
CPP=g++
CFLAGS=-std=c99 -pedantic -Wall
CPPFLAGS=-std=c++98 -pedantic -Wall
OCVFLAGS=`pkg-config --cflags opencv`
OCVLIBS=`pkg-config --libs opencv`
FILES=detector.cc  detector.h  erdil.cc  erdil.h  main.cpp  Makefile  parser.cc  parser.h  README.pdf  VIDEO0002.avi  VIDEO0003.avi

PRG=metection

.PHONY: all

all: $(PRG)

$(PRG): main.o parser.o detector.o erdil.o
	$(CPP) $(OCVLIBS) -o $@ $< parser.o detector.o erdil.o

main.o: main.cpp
	$(CPP) -c $(CPPFLAGS) $(OCVFLAGS) -o $@ $<

parser.o: parser.cc
	$(CPP) -c $(CPPFLAGS) -o $@ $<

detector.o: detector.cc
	$(CPP) -c $(CPPFLAGS) $(OCVFLAGS) -o $@ $<

erdil.o: erdil.cc
	$(CPP) -c $(CPPFLAGS) $(OCVFLAGS) -o $@ $<


clean:
	rm -rf $(PRG) *.o

pack:
	zip -r xlibos01.zip $(FILES)
	zip -r xpanac02.zip $(FILES)
	zip -r xnohej00.zip $(FILES)
