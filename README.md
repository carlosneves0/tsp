# tsp

The Traveling Salesman Problem solved in C using MPI and OpenMP

## Compile: `make`

## Run in the cluster: `make exec`

This will run with a problem of size N = 3. Change the size with:

```
make exec n=4
```

```
make exec n=10
```

N = 14 is the biggest problem we solved. It took a few minutes using all the cluster's resources.

All problems are generated to have the optimal solution: `0, 1, 2, ..., N-1, 0`.

## Run in the cluster with DEBUG output: `make debug`

Like `make exec` we can pick the size of the problem: `make debug n=10`.

All debug output is piped to the `stderr` stream. A useful way to debug the execution of this program is to:

```
make debug 2> stderr
cat stderr | grep main
cat stderr | grep master
cat stderr | grep slave
cat stderr | grep bcast
cat stderr | grep -E 'scatter\b'
```

### Run **locally** with DEBUG output: `make _exec`

This will use `mpiexec`'s `--oversubscribe` flag to force the creation of `p` processes. By default `p` is 3. But can be overridden with:

```
make _exec p=7
```
