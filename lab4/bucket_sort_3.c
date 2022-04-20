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

#define _XOPEN_SOURCE
#define BUCKET_SIZE 20
#define BUCKET_COUNT_MULTIPLIER 5
#define MIN_CHUNK_SIZE 1000

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


void parse_schedule_type(enum ScheduleType* type, const char* str) {
    if (same_str(str, "static")) {
        *type = S_STATIC;
    } else if (same_str(str, "dynamic")) {
        *type = S_DYNAMIC;
    } else if (same_str(str, "guided")) {
        *type = S_GUIDED;
    } else if (same_str(str, "runtime")) {
        *type = S_RUNTIME;
    } else {
        fprintf(stderr, "Unknown schedule type: %s\n", str);
        exit(2);
    }
}

void print_buckets(Bucket* buckets, int bucket_count) {
    int i, j;
    printf("Printing buckets...\n");
    for (i=0; i < bucket_count; ++i) {
        Bucket bucket = buckets[i];
        for(j=0; j < bucket.el_count; ++j) {
            printf("%f ", bucket.elements[j]);
        }
        printf("\n");
    }
}


void print_array(double* array, int array_size) {
    int i;
    for (i = 0; i < array_size; ++i) {
        printf("%lf ", array[i]);
    }
    printf("\n");
}


double random_array(double* array, int size) {
    double time0, time1, totalTime;
    int i;

    time0 = omp_get_wtime();
    #pragma omp parallel private(i)
    {
        unsigned short start_num = 0xFFFF / omp_get_num_threads() * omp_get_thread_num();
        unsigned short work[3] = {0, 0, start_num};
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


int calculate_bucket_count(int array_size) {
    return BUCKET_COUNT_MULTIPLIER * array_size / BUCKET_SIZE;
}


void add_node_to_bucket(Bucket* buckets, int bucket_count, double val) {
    int bucket_idx = get_bucket_idx(val, bucket_count);
    int el_in_buckets = buckets[bucket_idx].el_count;

    if (el_in_buckets >= BUCKET_SIZE) {
        printf("Max bucket size reached on idx: %d\n", bucket_idx);
        exit(1);
    }

    buckets[bucket_idx].elements[el_in_buckets] = val;
    ++buckets[bucket_idx].el_count;
}


double perform_bucketing(double* array, int array_size, Bucket** buckets_per_thread, int bucket_count) {
    int i;
    double time0, time1, totalTime;
    
    time0 = omp_get_wtime();
    #pragma omp parallel private(i)
    {
        Bucket* my_buckets = buckets_per_thread[omp_get_thread_num()];
        #pragma omp for schedule(guided, MIN_CHUNK_SIZE)
        for (i=0; i < array_size; ++i) {
            add_node_to_bucket(my_buckets, bucket_count, array[i]);
        }
    }
    time1 = omp_get_wtime();
    totalTime = time1 - time0;

    return totalTime;
}


double merge_buckets(Bucket** buckets_per_thread, int bucket_count, Bucket* main_buckets) {
    int i, j, k;
    double time0, time1, totalTime;
    
    time0 = omp_get_wtime();

    #pragma omp parallel private(i, j, k)
    {
        int thread_count = omp_get_num_threads();
        #pragma omp for schedule(guided, MIN_CHUNK_SIZE)
        for (i = 0; i < bucket_count; ++i) {
            for (j = 0; j < thread_count; ++j) {
                for(k = 0; k < buckets_per_thread[j][i].el_count; ++k) {
                    add_node_to_bucket(main_buckets, bucket_count, buckets_per_thread[j][i].elements[k]);
                    // printf("Adding: %lf, for i j k = {%d %d %d}\n", buckets_per_thread[j][i].elements[k], i, j, k);
                }
            }
        }
    }

    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    return totalTime;
}


void sort_single_bucket(Bucket* single_bucket) {
    int i, j;
    double key;
    double* array = single_bucket -> elements;

    for (i = 1; i < single_bucket -> el_count; ++i) {
        key = array[i];
        j = i - 1;
        while (j >= 0 && array[j] > key) {
            array[j + 1] = array[j];
            --j;
        }
        array[j + 1] = key;
    }
}


double sort_buckets(Bucket* buckets, int bucket_count) { 
    int i;
    double time0, time1, totalTime;

    time0 = omp_get_wtime();

    #pragma omp parallel for private(i) schedule(guided, MIN_CHUNK_SIZE)
    for (i=0; i < bucket_count; ++i) {
        sort_single_bucket(&buckets[i]);
    }

    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    return totalTime;
}


void seq_fill_elem_count(Bucket* buckets, int bucket_count) {
    int i;
    int counter = 0;
    for (i = 0; i < bucket_count; ++i) {
        buckets[i].bucket_size = counter;   // .bucket_size counts how many are there elements prior to this bucket
        counter += buckets[i].el_count;
    }
}


double buckets_to_array(double* array, Bucket* buckets, int bucket_count) {
    int i, j;
    int array_iter = 0;
    double time0, time1, totalTime;

    time0 = omp_get_wtime();

    // sequencally count how many elements are prior to each bucket
    seq_fill_elem_count(buckets, bucket_count); 

    #pragma omp parallel for private(i, j) schedule(guided, MIN_CHUNK_SIZE)
    for (i = 0; i < bucket_count; ++i) {
        int prior_to_this = buckets[i].bucket_size;
        for (j = 0; j < buckets[i].el_count; ++j) {
            array[prior_to_this + j] = buckets[i].elements[j];
        }
    }

    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    return totalTime;
}


// Should be used withing OpenMP
void bucket_sort(double* array, int array_size, double start, int verbose) {
    int bucket_count = calculate_bucket_count(array_size);
    int i;
    int thread_count = omp_get_max_threads();

    double time0, time1, totalTime;
    double init_time, bucketing_time, merging_time, sorting_time, rewriting_time, dealloc_time;

    // Allocation of buckets
    time0 = omp_get_wtime(); 
    Bucket* main_buckets = (Bucket*) calloc(bucket_count, sizeof(Bucket));
    Bucket** buckets_per_thread = (Bucket**) malloc(thread_count * sizeof(Bucket*));

    for (i=0; i < thread_count; ++i) {
        buckets_per_thread[i] = (Bucket*) calloc(bucket_count, sizeof(Bucket));
    }
    time1 = omp_get_wtime();
    init_time = time1 - time0;
    
    bucketing_time = perform_bucketing(array, array_size, buckets_per_thread, bucket_count);
    merging_time = merge_buckets(buckets_per_thread, bucket_count, main_buckets);
    sorting_time = sort_buckets(main_buckets, bucket_count);
    rewriting_time = buckets_to_array(array, main_buckets, bucket_count);

    // Deallocation of buckets
    time0 = omp_get_wtime();
    for (i=0; i < thread_count; ++i) {
        free(buckets_per_thread[i]);
    }
    free(main_buckets);
    time1 = omp_get_wtime();


    dealloc_time = time1 - time0;
    totalTime = time1 - start;
    if (verbose) {
        printf("Bucket allocation time: %lf\n", init_time);
        printf("Bucketing time: %lf\n", bucketing_time);  
        printf("Bucket merging time: %lf\n", merging_time);
        printf("Sorting buckets time: %lf\n", sorting_time);  
        printf("Rewriting buckets back to array: %lf\n", rewriting_time);
        printf("Bucket deallocation time: %lf\n", dealloc_time);
        printf("Total time: %lf\n", totalTime);
    } else {
        printf("%lf,", init_time);
        printf("%lf,", bucketing_time);  
        printf("%lf,", merging_time);
        printf("%lf,", sorting_time);  
        printf("%lf,", rewriting_time);
        printf("%lf,", dealloc_time);
        printf("%lf,", totalTime);
    }
}


int check_is_sorted(double* array, int size) {
    int i;
    for (i = 1; i < size; ++i) {
        if (array[i - 1] > array[i]) {
            return 0;   // false
        }
    }
    return 1;   // true
}


int main (int argc, char** argv) {
    double time0, randoming_time;
    int size, i;
    int verbose = 0;

    if (argc < 3) {
        fprintf(stderr, "expected: <number of processors> <array size> [--verbose]\n");
        exit(1);
    }
    
    omp_set_num_threads(atoi(argv[1]));
    size = atoi(argv[2]);

    if (argc == 4 && same_str(argv[3], "--verbose")) {
        verbose = 1;
    }

    double* array = (double*)malloc(size * sizeof(double));

    /* Początek obszaru równoległego. Tworzenie grupy wątków. Określanie zasięgu
    zmiennych*/
    time0 = omp_get_wtime();
    randoming_time = random_array(array, size);
    bucket_sort(array, size, time0, verbose);

    if (verbose) {
        printf("Randoming time: %lf\n", randoming_time);
        printf("Threads: %s\n", argv[1]);
        printf("Array size: %d\n", size);
        printf("Is sorted: %s\n", check_is_sorted(array, size) ? "yes" : "no");
    } else {
        printf("%lf,", randoming_time);
        printf("%s,", argv[1]);
        printf("%d,", size);
        printf("%d\n", check_is_sorted(array, size));
    }
    return 0;
}
