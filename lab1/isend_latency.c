#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char** argv) {
  MPI_Init(NULL, NULL);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // We are assuming at least 2 processes for this task
  if (world_size != 2) {
    fprintf(stderr, "World size must be equal 2 for %s\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1); 
  }

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <repeats> \n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1); 
  }
  
  int repeats = atoi(argv[1]);
  double start, end, time;
  unsigned long long i;

  if (world_rank == 0) {
    MPI_Barrier(MPI_COMM_WORLD);

    start = MPI_Wtime();
    for (i = 0; i < repeats; ++i) {
      MPI_Request request;
      MPI_Isend(NULL, 0, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &request);
      MPI_Wait(&request, MPI_STATUS_IGNORE);
    }
    end = MPI_Wtime();
    time = (end - start) / repeats;
    printf("Latency for Isend: %.15f\n", time);

  } else if (world_rank == 1) {

    MPI_Barrier(MPI_COMM_WORLD);
    for (i = 0; i < repeats; ++i) {
      MPI_Recv(NULL, 0, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }

  MPI_Finalize();
  return 0;
}
