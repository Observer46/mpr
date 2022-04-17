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
#define BUCKET_COUNT_MULTIPLIER 6
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


void random_array(double* array, int size) {
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
    printf("Randoming time: %lf\n", totalTime);
    // print_array(array, size);
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

    if(buckets[bucket_idx].bucket_size == el_in_buckets){
        printf("Max bucket size reached on idx: %d\n", bucket_idx);
        exit(1);
    }

    buckets[bucket_idx].elements[el_in_buckets] = val;
    ++buckets[bucket_idx].el_count;
}


void perform_bucketing(double* array, int array_size, Bucket* buckets, int bucket_count) {
    int i;
    double time0, time1, totalTime;
    
    // parallel or not - depends on version
    time0 = omp_get_wtime();
    for (i=0; i < array_size; ++i) {
        add_node_to_bucket(buckets, bucket_count, array[i]);
    }
    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    printf("Bucketing time: %lf\n", totalTime);  

    // print_buckets(buckets, bucket_count);
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


void sort_buckets(Bucket* buckets, int bucket_count) { 
    int i;
    double time0, time1, totalTime;

    time0 = omp_get_wtime();
    for (i=0; i < bucket_count; ++i) {
        sort_single_bucket(&buckets[i]);
    }
    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    printf("Sorting buckets time: %lf\n", totalTime);  
}


void buckets_to_array(double* array, Bucket* buckets, int bucket_count) {
    int i, j;
    int array_iter = 0;
    double time0, time1, totalTime;

    time0 = omp_get_wtime();
    for (i = 0; i < bucket_count; ++i) {
        for (j = 0; j < buckets[i].el_count; ++j) {
            array[array_iter++] = buckets[i].elements[j];
        }
    }
    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    printf("Rewriting buckets back to array: %lf\n", totalTime);
}


// Should be used withing OpenMP
void bucket_sort(double* array, int array_size) {
    int bucket_count = calculate_bucket_count(array_size);
    double time0, time1, totalTime;
    int i;

    // printf("Bucket count: %d\n", bucket_count);
    Bucket* buckets = (Bucket*) calloc(bucket_count, sizeof(Bucket));

    // Initialize buckets 
    for(i = 0; i < bucket_count; i++){
        buckets[i].el_count = 0;
        buckets[i].bucket_size = BUCKET_SIZE;
    }

    perform_bucketing(array, array_size, buckets, bucket_count);
    sort_buckets(buckets, bucket_count);
    buckets_to_array(array, buckets, bucket_count);
    // print_array(array, size);
}


int main (int argc, char** argv) {
    double time0, time1, totalTime;
    int size, i;

    if (argc < 3) {
        fprintf(stderr, "expected: <number of processors> <array size>\n");
        exit(1);
    }
    
    omp_set_num_threads(atoi(argv[1]));
    size = atoi(argv[2]);
    double* array = (double*)malloc(size * sizeof(double));

    /* Początek obszaru równoległego. Tworzenie grupy wątków. Określanie zasięgu
    zmiennych*/
    time0 = omp_get_wtime();

    random_array(array, size);
    bucket_sort(array, size);

    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    
    printf("Threads: %d\n", omp_get_max_threads());
    printf("Array size: %d\n", size);
    printf("Total time: %lf\n", totalTime);
    return 0;
}
