CFLAGS= -Wall -g -I./
LIBS=-lpthread
tiny: tiny.o csapp.o
	cc -o tiny tiny.o csapp.o $(LIBS)
	# cc -o echoclient echoclient.o csapp.o $(LIBS)
	# cc -o echoserveri echocserveri.p csapp.o $(LIBS)
	gcc -o ./cgi-bin/adder ./cgi-bin/adder.c $(LIBS)

echoclient: echoclient.o csapp.o
	cc -o echoclient echoclient.o csapp.o $(LIBS)

echoserveri: echoserveri.o csapp.o
	cc -o echoserveri echoserveri.o csapp.o $(LIBS)

clean:
	rm *.o tiny echoclient echoserveri
	# rm *.o tiny
	rm ./cgi-bin/adder
re: clean tiny echoclient echoserveri