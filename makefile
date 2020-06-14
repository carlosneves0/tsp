.DEFAULT_GOAL := build
.PHONY: build clean
.workdir := $(CURDIR)
.root := $(shell dirname '$(abspath $(lastword $(MAKEFILE_LIST)))')
.rootr := $(shell realpath --relative-to '$(.workdir)' '$(.root)')
.bin := $(.root)/.bin
.binr := $(shell realpath --relative-to '$(.workdir)' '$(.bin)')

build: $(.bin)/tsp.seq.rec

$(.bin)/tsp.seq.rec: $(.root)/main.c $(.rootr)/tsp.c | $(.bin)
	gcc $(.rootr)/main.c $(.rootr)/tsp.c -o $(.binr)/tsp.seq.rec -Wall

$(.bin)/tsp: $(.root)/main.c | $(.bin)
	mpicc $(.rootr)/main.c -o $(.binr)/tsp -Wall

$(.bin):
	mkdir -p $(.binr)

clean:
	rm -rf $(.binr)
