#include <stdio.h>
#include <stdlib.h>


#include "gputimer.h"

#define BLOCK_SIZE 32 

__global__ void matrix_transpose_naive(int *input, int *output, int matrix_size) {

	const int coef_x = gridDim.x * blockDim.x;
	const int coef_y = gridDim.y * blockDim.y;

	int indexX = threadIdx.x + blockIdx.x * blockDim.x;
	int indexY = threadIdx.y + blockIdx.y * blockDim.y + coef_y * (indexX / matrix_size);
	indexX %= matrix_size;

	while (indexX < matrix_size && indexY < matrix_size) {
		int index = indexY * matrix_size + indexX;
		int transposedIndex = indexX * matrix_size + indexY;

		output[transposedIndex] = input[index];

		indexX += coef_x;
		if (indexX >= matrix_size) {
			indexY += coef_y * (indexX / matrix_size);
			indexX %= matrix_size;
		}

		index = indexY * matrix_size + indexX;
		transposedIndex = indexX * matrix_size + indexY;
	}
}

__global__ void matrix_transpose_shared(int *input, int *output, int matrix_size) {

	__shared__ int sharedMemory [BLOCK_SIZE] [BLOCK_SIZE];
	
	const int coef_x = gridDim.x * blockDim.x;
	const int coef_y = gridDim.y * blockDim.y;

	// global index	
	int indexX = threadIdx.x + blockIdx.x * blockDim.x;
	int indexY = threadIdx.y + blockIdx.y * blockDim.y + coef_y * (indexX / matrix_size);
	indexX %= matrix_size;

	// local index
	const int localIndexX = threadIdx.x;
	const int localIndexY = threadIdx.y;

	while (indexX < matrix_size && indexY < matrix_size) {
		int index = indexY * matrix_size + indexX;
		int transposedIndex = indexX * matrix_size + indexY;
		
		sharedMemory[localIndexX][localIndexY] = input[index];

		__syncthreads();

		// writing into global memory in coalesed fashion via transposed data in shared memory
		output[transposedIndex] = sharedMemory[localIndexX][localIndexY];

		indexX += coef_x;
		if (indexX >= matrix_size) {
			indexY += coef_y * (indexX / matrix_size);
			indexX %= matrix_size;
		}

		index = indexY * matrix_size + indexX;
		transposedIndex = indexX * matrix_size + indexY;
	}
}

//basically just fills the array with index.
void fill_array(int *data, int matrix_size) {
	for(int idx=0;idx<(matrix_size * matrix_size);idx++)
		data[idx] = idx;
}

bool is_transposed(int *a, int *b, int matrix_size) {
	for (int i = 0; i < matrix_size* matrix_size; ++i) {
		int row = i / matrix_size;
		int col = i % matrix_size;

		if (a[row * matrix_size + col] != b[col * matrix_size + row]) {
			printf("Wrong value at %d %d: (%d) %d\n", row, col, a[row * matrix_size + col], b[col * matrix_size + row]);
			printf("Near: %d (%d), %d (%d)\n", b[i + 1], a[i + 1], b[i + 2], a[i + 2]);
			return false;
		}
	}
	return true;
} 

void print_output(int *a, int *b, int matrix_size) {
	printf("\n Original Matrix::\n");
	for(int idx=0;idx<(matrix_size*matrix_size);idx++) {
		if(idx % matrix_size == 0)
			printf("\n");
		printf(" %d ",  a[idx]);
	}
	printf("\n Transposed Matrix::\n");
	for(int idx=0;idx<(matrix_size*matrix_size);idx++) {
		if(idx % matrix_size == 0)
			printf("\n");
		printf(" %d ",  b[idx]);
	}
	printf("\n");
}

int main(int argc, char* argv[]) {
	int *a, *b;
        int *d_a, *d_b; // device copies of a, b, c

	if (argc != 4) {
		fprintf(stderr, "Usage %s <matrix_size> <blocks> <threads_per_block>\n", argv[0]);
		exit(1);
	}

	int matrix_size = atoi(argv[1]);
	int blocks = atoi(argv[2]);
	int threads_per_block = atoi(argv[3]);

	if (threads_per_block > BLOCK_SIZE) {
		fprintf(stderr, "Max allowed threads per block: 32\n");
		exit(2);
	}

	printf("%d %d %d ", matrix_size, blocks, threads_per_block);

	int size = matrix_size * matrix_size * sizeof(int);
	cudaError_t returnValue;

	// Alloc space for host copies of a, b, c and setup input values
	a = (int *)malloc(size); fill_array(a, matrix_size);
	b = (int *)malloc(size);

	// Alloc space for device copies of a, b, c
	returnValue = cudaMalloc((void **)&d_a, size);
	// printf("%s ", cudaGetErrorString(returnValue));
	returnValue = cudaMalloc((void **)&d_b, size);
	// printf("%s ", cudaGetErrorString(returnValue));

	// Copy inputs to device
	returnValue = cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice);
	// printf("%s ", cudaGetErrorString(returnValue));
	returnValue = cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice);
	// printf("%s\n", cudaGetErrorString(returnValue));

	dim3 blockSize(threads_per_block, threads_per_block,1);
	dim3 gridSize(blocks / 5, 5, 1);

	GpuTimer timer;
	cudaError_t err = cudaSuccess;

	timer.Start();
	matrix_transpose_naive<<<gridSize, blockSize>>>(d_a, d_b, matrix_size);
	timer.Stop();
	
	printf("%f ", timer.Elapsed());

	err = cudaGetLastError();

	if (err != cudaSuccess) {
		fprintf(stderr, "1: Failed to launch error kernel (error code %s)!\n",
				cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	// Copy result back to host
	cudaMemcpy(b, d_b, size, cudaMemcpyDeviceToHost);

	if (!is_transposed(a, b, matrix_size)) {
		fprintf(stderr, "Naive transpose failed!\n");
	}
	// print_output(a, b, matrix_size);

	timer.Start();
	matrix_transpose_shared<<<gridSize, blockSize>>>(d_a, d_b, matrix_size);
	timer.Stop();

	printf("%f\n", timer.Elapsed());

	err = cudaGetLastError();
	if (err != cudaSuccess) {
		fprintf(stderr, "2: Failed to launch error kernel (error code %s)!\n",
				cudaGetErrorString(err));
		exit(EXIT_FAILURE);
	}

	// Copy result back to host
	cudaMemcpy(b, d_b, size, cudaMemcpyDeviceToHost);
	if (!is_transposed(a, b, matrix_size)) {
		fprintf(stderr, "Shared transpose failed!\n");
	}

	// terminate memories
	free(a);
	free(b);
    cudaFree(d_a);
	cudaFree(d_b); 

	return 0;
}
