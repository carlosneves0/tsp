.DEFAULT_GOAL := build
.PHONY: build arquivo-entrada.txt clean exec debug cluster-topology genprobl _exec

build: pcv arquivo-entrada.txt

pcv: .bin/tsp
	ln -sf ./.bin/tsp pcv

.bin/tsp: .bin/compute.o .bin/list.o .bin/main.o .bin/master.o .bin/message.o .bin/slave.o .bin/tsp.o | .bin
	mpicc .bin/compute.o .bin/list.o .bin/main.o .bin/master.o .bin/message.o .bin/slave.o .bin/tsp.o -o .bin/tsp -fopenmp
.bin/tsp.debug: .bin/compute.o .bin/list.o .bin/main.debug.o .bin/master.debug.o .bin/message.o .bin/slave.debug.o .bin/tsp.o | .bin
	mpicc .bin/compute.o .bin/list.o .bin/main.debug.o .bin/master.debug.o .bin/message.o .bin/slave.debug.o .bin/tsp.o -o .bin/tsp.debug -fopenmp

.bin/compute.o: compute.h compute.c | .bin
	mpicc -c compute.c -o .bin/compute.o -Wall

.bin/list.o: list.h list.c | .bin
	mpicc -c list.c -o .bin/list.o -Wall

.bin/main.o: debug.h master.h slave.h main.c | .bin
	mpicc -c main.c -o .bin/main.o -Wall
.bin/main.debug.o: debug.h master.h slave.h main.c | .bin
	mpicc -c main.c -o .bin/main.debug.o -Wall -DDEBUG

.bin/master.o: debug.h list.h message.h tsp.h master.c | .bin
	mpicc -c master.c -o .bin/master.o -Wall
.bin/master.debug.o: debug.h list.h message.h tsp.h master.c | .bin
	mpicc -c master.c -o .bin/master.debug.o -Wall -DDEBUG

.bin/message.o: message.h message.c | .bin
	mpicc -c message.c -o .bin/message.o -Wall

.bin/slave.o: debug.h message.h slave.h tsp.h slave.c | .bin
	mpicc -c slave.c -o .bin/slave.o -Wall
.bin/slave.debug.o: debug.h message.h slave.h tsp.h slave.c | .bin
	mpicc -c slave.c -o .bin/slave.debug.o -Wall -DDEBUG

.bin/tsp.o: list.h tsp.h tsp.c | .bin
	mpicc -c tsp.c -o .bin/tsp.o -Wall

.bin:
	mkdir -p .bin

ifndef p
p := 0
endif
arquivo-entrada.txt: __problems__/$(p)/input
	ln -sf ./__problems__/$(p)/input arquivo-entrada.txt

clean:
	rm -rf .bin

exec: nodes.txt .bin/tsp arquivo-entrada.txt
	mpiexec --map-by ppr:1:node --hostfile nodes.txt .bin/tsp arquivo-entrada.txt

debug: nodes.txt .bin/tsp.debug arquivo-entrada.txt
	mpiexec --map-by ppr:1:node --hostfile nodes.txt .bin/tsp.debug arquivo-entrada.txt

cluster-topology:
	bash __scripts__/cluster-topology.bash

n := $(n)
genprobl:
	@exec bash __scripts__/genprobl.bash $(n)

# LOCAL exec
_exec: .bin/tsp.debug arquivo-entrada.txt
	mpiexec --np 3 --oversubscribe .bin/tsp.debug arquivo-entrada.txt
