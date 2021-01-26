CFLAGS= -Wall -g -I./
LIBS=-lpthread
tiny: tiny.o csapp.o
	cc -o tiny tiny.o csapp.o $(LIBS)
	gcc -o ./cgi-bin/adder ./cgi-bin/adder.c $(LIBS)
clean:
	rm *.o tiny
	rm ./cgi-bin/adder
re: clean tiny