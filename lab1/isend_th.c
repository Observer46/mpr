#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

float convert_bytes_to_mbit(unsigned long long byte_count) {
  return 1.f * byte_count / (1024 * 1024) * 8;
}

void fill_buf(char* buf, unsigned long long size) {
  unsigned long long i;
  for (i = 0; i < size; ++i) {
    buf[i] = 'a';
  }
}

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

  if (argc != 5) {
    fprintf(stderr, "Usage: %s <starting size [KB]> <max_size [KB]> <increment [KB]> <repeats>\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1); 
  }

  int starting_size = atoi(argv[1]);
  int max_size      = atoi(argv[2]);
  int inc           = atoi(argv[3]);
  int size = starting_size;
  int repeats       = atoi(argv[4]);
  double start, end, time;
  char* buf;
  
  for (; size <= max_size; size += inc) {
    buf = (char*) calloc(size, sizeof(char));
    int i;

    if (world_rank == 0) {
      fill_buf(buf, size);
      MPI_Barrier(MPI_COMM_WORLD);

      start = MPI_Wtime();
      for (i = 0; i < repeats; ++i) {
        MPI_Request request;
        MPI_Isend(buf, size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
      }
      end = MPI_Wtime();
      time = (end - start) / repeats;
      float throttle = 1.f * convert_bytes_to_mbit(size) / time;
      
      printf("%d %f %.1f\n", size, time, throttle);

    } else if (world_rank == 1) {

      MPI_Barrier(MPI_COMM_WORLD);
      for (i = 0; i < repeats; ++i) {
        MPI_Recv(buf, size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }
    free(buf);
  }

  MPI_Finalize();
  return 0;
}
