.DEFAULT_GOAL := build
.PHONY: build clean exec _exec cluster-topology genprobl

build: pcv arquivo-entrada.txt

pcv: .bin/tsp
	ln -sf ./.bin/tsp pcv

.bin/tsp: .bin/list.o .bin/tsp.o .bin/main.o | .bin
	mpicc .bin/list.o .bin/tsp.o .bin/main.o -o .bin/tsp -fopenmp

.bin/list.o: list.h list.c | .bin
	mpicc -c list.c -o .bin/list.o -Wall

.bin/tsp.o: tsp.h tsp.c | .bin
	mpicc -c tsp.c -o .bin/tsp.o -Wall

.bin/main.o: debug.h main.c | .bin
	mpicc -c main.c -o .bin/main.o -Wall

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
