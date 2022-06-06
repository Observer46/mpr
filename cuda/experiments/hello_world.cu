#include <stdio.h>
#include <stdlib.h>

#define NUM_BLOCKS 16
#define BLOCK_WIDTH 2

__global__ void hello(void) {
    printf("Dim grid: %d %d %d\n", gridDim.x, gridDim.y, gridDim.z);
    printf("Hello World! from thread [%d, %d] From device\n", threadIdx.x, blockIdx.x);
}

int main() {
    hello<<<NUM_BLOCKS, BLOCK_WIDTH>>>();
    cudaDeviceSynchronize();
    printf("ALL DONE!\n");
    return 0;
}