.DEFAULT_GOAL := build
.PHONY: build clean exec _exec cluster-topology genprobl

build: pcv arquivo-entrada.txt

pcv: .bin/tsp
	ln -sf ./.bin/tsp pcv

.bin/tsp: .bin/compute.o .bin/list.o .bin/main.o .bin/master.o .bin/message.o .bin/slave.o .bin/tsp.o | .bin
	mpicc .bin/compute.o .bin/list.o .bin/main.o .bin/master.o .bin/message.o .bin/slave.o .bin/tsp.o -o .bin/tsp -fopenmp

.bin/compute.o: compute.h compute.c | .bin
	mpicc -c compute.c -o .bin/compute.o -Wall

.bin/list.o: list.h list.c | .bin
	mpicc -c list.c -o .bin/list.o -Wall

.bin/main.o: debug.h master.h slave.h main.c | .bin
	mpicc -c main.c -o .bin/main.o -Wall

.bin/master.o: debug.h list.h message.h tsp.h master.c | .bin
	mpicc -c master.c -o .bin/master.o -Wall

.bin/message.o: message.h message.c | .bin
	mpicc -c message.c -o .bin/message.o -Wall

.bin/slave.o: debug.h message.h slave.h tsp.h slave.c | .bin
	mpicc -c slave.c -o .bin/slave.o -Wall

.bin/tsp.o: list.h tsp.h tsp.c | .bin
	mpicc -c tsp.c -o .bin/tsp.o -Wall

.bin:
	mkdir -p .bin

arquivo-entrada.txt: __problems__/1/input
	ln -sf ./__problems__/1/input arquivo-entrada.txt

clean:
	rm -rf .bin

exec: nodes.txt .bin/tsp arquivo-entrada.txt
	mpiexec --map-by ppr:1:node --hostfile nodes.txt .bin/tsp arquivo-entrada.txt

_exec: .bin/tsp arquivo-entrada.txt
	mpiexec --np 3 --oversubscribe .bin/tsp arquivo-entrada.txt
	@rm arquivo-entrada.txt

cluster-topology:
	bash __scripts__/cluster-topology.bash

n := $(n)
genprobl:
	@exec bash __scripts__/genprobl.bash $(n)
