/* 
    Wymyślenie algorytmu równoległego dla bucket sorta - złożoność liniowa
    Może być inna złożoność równoległa stworzonych algorytmów. 
    
    Należy:
    - Stworzyć strukturę danych - najlepiej tablica jednowymiarowa
    - Zrównoleglanie pętli for, klauzula schedule (przydział pętli do odpowiednich wątków) 
            -> wygenerowanie losowych liczb do tablicy
    - sprawdzenie, że dodanie wątku powoduje przyspieszenie (sprawdzić jak dyrektywa jest podzielona na wątki,
     problem z generatorem- musi być odporna na wątki)

    - Ocenić algorytmy według poznanych metryk (wybranie metryk)
    
*/

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define _XOPEN_SOURCE
#define BUCKET_SIZE 20
#define BUCKET_COUNT_MULTIPLIER 5
#define MIN_CHUNK_SIZE 1000
#define MAX_IN_BUCKET 100

enum ScheduleType {S_STATIC, S_DYNAMIC, S_GUIDED, S_RUNTIME};


struct Bucket
{
    double* elements;
    int el_count;
    int bucket_size;
};
typedef struct Bucket Bucket;


int same_str(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}


void print_array(double* array, int array_size) {
    int i;
    for (i = 0; i < array_size; ++i) {
        printf("%lf ", array[i]);
    }
    printf("\n");
}


void random_init(unsigned short* work) {  // len 3
    unsigned short start_num = 0xFFFF / omp_get_num_threads() * omp_get_thread_num();
    work[0] = 0;
    work[1] = 0;
    work[2] = start_num;
}


double random_array(double* array, int size, int random_type) {
    double time0, time1, totalTime;
    int i;

    time0 = omp_get_wtime();
    #pragma omp parallel private(i)
    {
        unsigned short work[3];
        random_init(work);    

        #pragma omp for schedule(guided, MIN_CHUNK_SIZE)
            for (i=0; i < size; ++i)
                array[i] = erand48(work);
    }
    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    return totalTime;
}


int get_bucket_idx(double val, int bucket_count) {
    return (int) (bucket_count * val);
}


int max_f(int a, int b) {
    return a > b ? a : b;
}


int analyze_for_bucket_count(double* array, int array_size, int bucket_count, int* bucket_element_counts) {
    int i;
    int* buckets = (int*) calloc(bucket_count, sizeof(int));

    for (i = 0; i < array_size; ++i) {
        ++buckets[get_bucket_idx(array[i], bucket_count)];
    }

    int max_val;
    double var, mean;

    max_val = 0;
    var = 0;
    mean = array_size / bucket_count;

    for (i = 0; i < bucket_count; ++i) {
        ++bucket_element_counts[buckets[i]];
        max_val = max_f(max_val, buckets[i]);
        var += (mean - buckets[i]) * (mean - buckets[i]);
    }
    
    free(buckets);
    var /= bucket_count;
    return max_val;
}


void analyze_mean_and_var(double* array, int array_size) {
    int i;
    double mean = 0.0, var = 0.0;

    for (i = 0; i < array_size; ++i) {
        mean += array[i];
    }
    mean /= array_size;

    for (i = 0; i < array_size; ++i) {
        var += (mean - array[i]) * (mean - array[i]);
    }
    var /= array_size;
}


// Returns if buckets would be overflown
int measure_uniformness(double* array, int array_size, double m, int bucket_size, int* bucket_element_counts) {
    int max_in_bucket = analyze_for_bucket_count(array, array_size, (int) (m * array_size / bucket_size), bucket_element_counts);
    analyze_mean_and_var(array, array_size);
    return max_in_bucket > bucket_size;
}


int main (int argc, char** argv) {
    double time0, randoming_time;
    int array_size, bucket_size, i;
    double m;
    int test_repeats;

    srand(time(NULL));
    int bucket_element_counts[MAX_IN_BUCKET] = { 0 };

    if (argc < 6) {
        fprintf(stderr, "expected: <number of processors> <array size> <bucket multiplier> <bucket_size> <test repeats>\n");
        exit(1);
    }

    array_size = atoi(argv[2]);
    printf("Threads: %s\n", argv[1]);
    printf("Array size: %d\n", array_size);
    printf("Bucket count multiplier: %s\n", argv[3]);
    printf("Bucket size: %s\n\n", argv[4]);
    
    omp_set_num_threads(atoi(argv[1]));
    sscanf(argv[3] , "%lf", &m);
    bucket_size = atoi(argv[4]);

    test_repeats = atoi(argv[5]);
    double* array = (double*)malloc(array_size * sizeof(double));

    for (i = 0 ; i < test_repeats; ++i) {
        random_array(array, array_size, 1);
        measure_uniformness(array, array_size, m, bucket_size, bucket_element_counts);
    }

    for (i = 0; i < MAX_IN_BUCKET; ++i) {
        if (bucket_element_counts[i] > 0) {
            printf("Buckets with %d elements: %lf\n", i, 1.0 * bucket_element_counts[i] / test_repeats);
        }
    }
    return 0;
}
