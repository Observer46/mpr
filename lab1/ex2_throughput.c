#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TEST_TRIES 2000


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
  
  double start, end, time;
  char* buf;
  int bufsize;
  
  for (unsigned i = 4; i <= 8; ++i) {
    bufsize = 1 << i;
    buf = (char*) malloc(bufsize);

    if (world_rank == 0) {

      MPI_Barrier();

      start = MPI_Wtime();
      for (unsigned j = 0; j < TEST_TRIES; ++j) {
        MPI_Send(buf, bufsize, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
      }
      end = MPI_Wtime();
      time = (end - start) / TEST_TRIES;
      printf("Process 0 - sent %d bytes of data in time %f!\n", strlen(buf), time);

      MPI_Barrier();
      for (unsigned j = 0; j < TEST_TRIES; ++j) {
        MPI_Recv(buf, bufsize, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
      printf("Process 0 received data of length %d from process 0\n", strlen(buf));

    } else if (world_rank == 1) {

      MPI_Barrier();
      for (unsigned j = 0; j < TEST_TRIES; ++j) {
        MPI_Recv(buf, bufsize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
      printf("Process 1 received data of length %d from process 0\n", strlen(buf));

      MPI_Barrier();

      start = MPI_Wtime();
      for (unsigned j = 0; j < TEST_TRIES; ++j) {
        MPI_Send(buf, bufsize, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
      }
      end = MPI_Wtime();
      time = (end - start) / TEST_TRIES;
      printf("Process 1 - sent %d bytes of data in time %f!\n", strlen(buf), time);
    }
  }

  MPI_Finalize();
  return 0;
}
