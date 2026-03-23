CC = gcc
CFLAGS = -Wall -pthread -lrt

all: sensor processor display logger main

sensor:
	$(CC) sensor.c -o sensor $(CFLAGS)

processor:
	$(CC) processor.c -o processor $(CFLAGS)

display:
	$(CC) display.c -o display $(CFLAGS)

logger:
	$(CC) logger.c -o logger

main:
	$(CC) main.c -o main

run: all
	./main

clean:
	rm -f sensor processor display logger main
	rm -f environment_log.txt
