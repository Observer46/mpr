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
#define BUCKET_SIZE 10

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

void random_array(double* array, int size) {
    int i;
    unsigned short work1[3] = {0, 0, 0};
    unsigned short work2[3] = {0xFF00, 0, 0};
    unsigned short work3[3] = {0x00FF, 0, 0};
    unsigned short work4[3] = {0, 0xFF00, 0};
    unsigned short work5[3] = {0, 0x00FF, 0 };
    printf("1:%lf 2:%lf 3:%lf 4:%lf 5:%lf \n", erand48(work5), erand48(work2), erand48(work1),  erand48(work3), erand48(work4));
    exit(0);
    #pragma omp parallel private(i)
    {
        unsigned short work[3] = {omp_get_thread_num(), omp_get_thread_num(), omp_get_thread_num()};
            #pragma omp for schedule(guided)
                for (i=0; i < size; ++i)
                    array[i] = erand48(work);
    }
}

int get_bucket_idx(double val, int bucket_count) {
    return (int) (bucket_count * val);
}

int calculate_bucket_count(int array_size) {
    return 4 * array_size / BUCKET_SIZE;    // 100% more than expected
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

// Should be used withing OpenMP
void bucket_sort(double* array, int size) {
    int thread_count = omp_get_max_threads();
    int bucket_count = calculate_bucket_count(size);
    printf("Bucket count: %d\n", bucket_count);
    double time0, time1, totalTime;
    int i;

    Bucket* buckets = (Bucket*) calloc(bucket_count, sizeof(Bucket));

    // Initialize buckets 
    for(i = 0; i < bucket_count; i++){
        buckets[i].el_count = 0;
        buckets[i].bucket_size = BUCKET_SIZE;
    }

    // print_buckets(buckets, BUCKET_NUMBER);
    
    time0 = omp_get_wtime();
    for (i=0; i < size; ++i) {
        add_node_to_bucket(buckets, bucket_count, array[i]);
    }
    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    printf("Bucketing time: %lf\n", totalTime);  

    print_buckets(buckets, bucket_count);
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


    time0 = omp_get_wtime();
    /* Początek obszaru równoległego. Tworzenie grupy wątków. Określanie zasięgu
    zmiennych*/

    random_array(array, size);
    bucket_sort(array, size);

    time1 = omp_get_wtime();
    totalTime = time1 - time0;
    
//    for(i=0; i < size; ++i) {
//        printf("%f ", array[i]);
//    }
//    printf("\n");
    printf("Total time: %d %d %lf\n", omp_get_max_threads(), size, totalTime);

    return 0;
}
