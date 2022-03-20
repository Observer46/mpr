#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define SEED_OFFSET 1913

float randf() {
  return (float) rand() / RAND_MAX;
}

int is_in_circle(float x, float y, float r) {
  return x * x + y * y < r * r;
}

unsigned long long monte_carlo_pi(unsigned long long point_count, float r) {
  unsigned long long in_circle = 0L;
  unsigned long long i;

  for (i = 0L; i < point_count; ++i) {
    float x = randf(), y = randf();

    in_circle += is_in_circle(x, y, r);
  }

  return in_circle;
}

double calculate_pi(unsigned long long in_circle, unsigned long long point_count) {
  return 4.0 * in_circle / point_count; 
}

int main(int argc, char** argv) {
  MPI_Init(NULL, NULL);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  
  if (argc < 3) {
    fprintf(stderr, "required argument: <point count> <radius>");
    MPI_Abort(MPI_COMM_WORLD, 1); 
  }

  srand(world_rank + SEED_OFFSET);

  float r;
  double d;  

  sscanf(argv[1] , "%lf", &d);
  sscanf(argv[2] , "%f", &r);

  unsigned long long point_count = d;

  unsigned long long points_per_node = point_count / world_size;
  unsigned long long in_circle;

  if (world_rank == 0) {    
    printf("Point count: %lld\n", point_count);
    printf("Processor count: %d\n", world_size);
    double start, end, time;
    unsigned long long total_in_circle;

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    in_circle = monte_carlo_pi(points_per_node, r);
    MPI_Reduce(&in_circle, &total_in_circle, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    end = MPI_Wtime();
    time = end - start;

    printf("Total time: %f\n", time);
    printf("Total in circle: %lld\n", total_in_circle);
    printf("PI value: %lf\n", calculate_pi(total_in_circle, point_count));
  } else {
    MPI_Barrier(MPI_COMM_WORLD);
    in_circle = monte_carlo_pi(points_per_node, r);
    MPI_Reduce(&in_circle, NULL, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  }
  MPI_Finalize();
  
  return 0;
}
