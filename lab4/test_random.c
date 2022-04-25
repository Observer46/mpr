#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define _XOPEN_SOURCE
#define SIZE 1000000
#define BUCKETS 1000000

int get_bucket_idx(double val) {
    return BUCKETS * val;
}

int max_f(int a, int b) {
    return a > b ? a : b;
}

int analyze_for_bucket_count(double* array) {
    int i;
    int* buckets = (int*)calloc(BUCKETS, sizeof(int));

    int max_val = 0;
    int bucket_idx;

    for (i = 0; i < SIZE; ++i) {
        bucket_idx = get_bucket_idx(array[i]);
        ++buckets[bucket_idx];
        max_val = max_f(max_val, buckets[bucket_idx]);
    }
    free(buckets);
    printf("Max in bucket: %d\n", max_val);
    return max_val;
}

short rand_s() {
    return (short) rand();
}

int main(int argc, char** argv) {
    srand(time(NULL));

    double* things = (double*) malloc(sizeof(double) * SIZE);
    int i;
    unsigned short work[3] = {rand_s(), rand_s(), rand_s()};
    unsigned short test[7] = {0, 0, 0, rand_s(), rand_s(), rand_s(), rand_s()};
    lcong48(test);
 
    printf("Starting!\n");
    for (i = 0; i < SIZE; ++i) {
        things[i] =  erand48(work);
        // printf("%lf\n", things[i]);
    }
    analyze_for_bucket_count(things);
    free(things);
    return 0;
}