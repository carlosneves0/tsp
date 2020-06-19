# tsp
The Traveling Salesman Problem solved in C using MPI and OpenMP

TODO: rewrite this.

```
--- PSEUDOCODE ---
if (my_rank == MPI_MASTER)
{
  tsp_t* problem = input(argc, argv);

  nproc = ask_mpi();

  spawn(nproc);

  ncores = gather_ncores();

  bcast(cost_matrix);

  tsp_t* subproblems = split(problem, nproc, ncores);

  tsp_t* my_subproblems = scatterv(subproblems);

  path_t* my_subsolutions;

  #pragma omp shit
  my_subsolutions[i] = tsp_solve(my_subproblems[i]);

  path_t* all_subsolutions = gatherv(my_subsolutions);

  path_t* solution = merge(all_subsolutions);

  output(solution);
}
else
{
  gather_ncores();

  bcast(cost_matrix);

  tsp_t* my_subproblems = scatterv(subproblems);

  path_t* my_subsolutions;

  #pragma omp shit
  my_subsolutions[i] = tsp_solve(my_subproblems[i]);

  gatherv(my_subsolutions);
}
--- PSEUDOCODE ---
```
