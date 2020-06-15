.DEFAULT_GOAL := build
.PHONY: build clean

build: pcv arquivo-entrada.txt

pcv: .bin/tsp
	ln -sf ./.bin/tsp pcv

.bin/tsp: .bin/queue.o .bin/tsp.o .bin/main.o | .bin
	mpicc .bin/queue.o .bin/tsp.o .bin/main.o -o .bin/tsp

.bin/queue.o: queue.h queue.c | .bin
	mpicc -c queue.c -o .bin/queue.o -Wall

.bin/tsp.o: tsp.h tsp.c | .bin
	mpicc -c tsp.c -o .bin/tsp.o -Wall

.bin/main.o: main.c | .bin
	mpicc -c main.c -o .bin/main.o -Wall

.bin:
	mkdir -p .bin

arquivo-entrada.txt: __problems__/4/0
	ln -sf ./__problems__/4/0 arquivo-entrada.txt

clean:
	rm -rf .bin
