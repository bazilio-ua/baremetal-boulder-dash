INC=-I/opt/local/include

CFLAGS = -g -Wall $(INC)

LIBS=-L/opt/local/lib -lSDL2


OBJECTS = util.o frame_buffer.o boulder_dash.o

all: boulder-dash

boulder-dash: $(OBJECTS)
	gcc $(OBJECTS) -o boulder-dash $(LIBS)

util.o: ./util.c
	gcc -c ./util.c $(CFLAGS);

frame_buffer.o: ./frame_buffer.c
	gcc -c ./frame_buffer.c $(CFLAGS);

boulder_dash.o: ./boulder_dash.c
	gcc -c ./boulder_dash.c $(CFLAGS);

clean:
	rm -f *.o

purge:	clean
	rm -f boulder-dash
