.DEFAULT_GOAL := build
.PHONY: build arquivo-entrada.txt clean exec debug cluster-topology genproblems _exec

# @ANYWHERE
build: pcv arquivo-entrada.txt

pcv: .bin/tsp
	ln -sf ./.bin/tsp pcv

.bin/tsp: .bin/compute.o .bin/list.o .bin/main.o .bin/master.o .bin/message.o .bin/slave.o .bin/tsp.o | .bin
	mpicc .bin/compute.o .bin/list.o .bin/main.o .bin/master.o .bin/message.o .bin/slave.o .bin/tsp.o -o .bin/tsp -fopenmp
.bin/tsp.debug: .bin/compute.o .bin/list.o .bin/main.debug.o .bin/master.debug.o .bin/message.o .bin/slave.debug.o .bin/tsp.o | .bin
	mpicc .bin/compute.o .bin/list.o .bin/main.debug.o .bin/master.debug.o .bin/message.o .bin/slave.debug.o .bin/tsp.o -o .bin/tsp.debug -fopenmp

.bin/compute.o: compute.h compute.c | .bin
	mpicc -c compute.c -o .bin/compute.o -fopenmp -Wall

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

ifndef n
n := 3
endif
arquivo-entrada.txt: __problems__/$(n)/input
	ln -sf ./__problems__/$(n)/input arquivo-entrada.txt

__problems__/$(n)/input: | __problems__/$(n)
	bash __scripts__/genproblem.bash $(n) > __problems__/$(n)/input

__problems__/$(n):
	mkdir -p __problems__/$(n)

# @ANYWHERE
clean:
	rm -rf .bin

# @CLUSTER
exec: nodes.txt .bin/tsp arquivo-entrada.txt
	mpiexec --map-by ppr:1:node --hostfile nodes.txt .bin/tsp arquivo-entrada.txt

# @CLUSTER
debug: nodes.txt .bin/tsp.debug arquivo-entrada.txt
	mpiexec --map-by ppr:1:node --hostfile nodes.txt .bin/tsp.debug arquivo-entrada.txt

# @CLUSTER
cluster-topology:
	bash __scripts__/cluster-topology.bash

# @ANYWHERE
genproblem:
	@exec bash __scripts__/genproblems.bash $(n)

# @HOME
_exec: .bin/tsp.debug arquivo-entrada.txt
	mpiexec --np 2 --oversubscribe .bin/tsp.debug arquivo-entrada.txt
