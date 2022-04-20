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
#define MIN_CHUNK_SIZE 100

enum ScheduleType {S_STATIC, S_DYNAMIC, S_GUIDED, S_RUNTIME};

struct Bucket
{
    double elements[BUCKET_SIZE];
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


void random_init_1(unsigned short* work) {  // len 3
    unsigned short start_num = 0xFFFF / omp_get_num_threads() * omp_get_thread_num();
    work[0] = 0;
    work[1] = 0;
    work[2] = start_num;
}


unsigned short random_short() {
    return rand() % USHRT_MAX;
}


void random_init_2(unsigned short* work) {  // len 3
    work[0] = random_short();
    work[1] = random_short();
    work[2] = random_short();
}


void random_init_3(unsigned short* work) {
    FILE* urandom = fopen ("/dev/urandom", "r");
    setvbuf (urandom, NULL, _IONBF, 0);  // turn off buffering

    // fgetc() returns a `char`, we need to fill a `short`
    work[0] = (fgetc (urandom) << 8) | fgetc (urandom);
    work[1] = (fgetc (urandom) << 8) | fgetc (urandom);
    work[2] = (fgetc (urandom) << 8) | fgetc (urandom);

    // cleanup urandom
    fclose (urandom);
}


double random_array(double* array, int size, int random_type) {
    double time0, time1, totalTime;
    int i;

    time0 = omp_get_wtime();
    #pragma omp parallel private(i)
    {
        unsigned short work[3];

        if (random_type == 1) {
            random_init_1(work);
        } else if (random_type == 2) {
            random_init_2(work);
        } else if (random_type == 3) {
            random_init_3(work);
        }

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


int min_f(int a, int b) {
    return a < b ? a : b;
}


int analyze_for_bucket_count(double* array, int array_size, int bucket_count) {
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
        max_val = max_f(max_val, buckets[i]);
        var += (mean - buckets[i]) * (mean - buckets[i]);
    }
    var /= bucket_count;
    printf("Max in bucket: %d, bucket size variance: %lf\n", max_val, var);
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

    printf("Array mean: %lf, array variance: %lf\n", mean, var);
}


// Returns if buckets would be overflown
int measure_uniformness(double* array, int array_size, double m, int bucket_size, int method_no) {
    printf("Method %d:\n", method_no);
    int max_in_bucket = analyze_for_bucket_count(array, array_size, (int) (m * array_size / bucket_size));
    analyze_mean_and_var(array, array_size);
    printf("\n");
    return max_in_bucket > bucket_size;
}


int main (int argc, char** argv) {
    double time0, randoming_time;
    int array_size, bucket_size, i;
    double m;
    int test_repeats;

    srand(time(NULL));

    if (argc < 6) {
        fprintf(stderr, "expected: <number of processors> <array size> <bucket multiplier> <bucket_size> <test repeats>\n");
        exit(1);
    }

    array_size = atoi(argv[2]);
    printf("Threads: %s\n", argv[1]);
    printf("Array size: %d\n", array_size);
    
    omp_set_num_threads(atoi(argv[1]));
    sscanf(argv[3] , "%lf", &m);
    bucket_size = atoi(argv[4]);

    test_repeats = atoi(argv[5]);
    double* array = (double*)malloc(array_size * sizeof(double));

    int fail = 0;

    for (i = 0; i < test_repeats; ++i) {
        random_array(array, array_size, 1);
        fail = max_f(fail, measure_uniformness(array, array_size, m, bucket_size, 1));

        if (fail) break;
    }
    
    printf("Method: %s\n", fail ? "failed" : "succeded");
    printf("------------------\n");
    return 0;
}
