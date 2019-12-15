#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <ctime>

#define THREADS 256
#define BLOCKS 32768
#define NUM THREADS * BLOCKS

bool check(int *array, int length) {
	bool result = true;
	for (int i = 0; i < length - 1; i++) {
		if (array[i] > array[i + 1]) {
		    return false;
		}
	}
	return result;
}

void bitonic(int* array, int length) {
	int K = log2(length);
	int d = 1 << K;
    K--;
	for (int n = 0; n < d >> 1; n++) {
        if (array[n] > array[d - n - 1]) {
            int temp = array[n];
            array[n] = array[d - n - 1];
            array[d - n - 1] = temp;
        }
    }
	for (int k = K; k > 0; k--) {
		d = 1 << k;
		for (int m = 0; m < length; m += d) {
		    for (int n = 0; n < d >> 1; n++) {
                if (array[m + n] > array[m + (d >> 1) + n]) {
                    int temp = array[m + n];
                    array[m + n] = array[m + (d >> 1) + n];
                    array[m + (d >> 1) + n] = temp;
                }
            }
        }
	}
}

void bitonic_sort(int* array, int length) {
    int* arr = (int*)malloc(sizeof(int) * length);
	for (int n = 0; n < length; n++)
		arr[n] = array[n];
	int K = log2(length);
	for (int k = 1; k <= K; k++) {
		for (int n = 0; n < length; n += 1 << k) {
            int* arr_ptr = &arr[n];
			bitonic(arr_ptr, 1 << k);
		}
	}
	for (int n = 0; n < length; n++) {
		array[n] = arr[n];
	}
	free(arr);
}

__global__ void bitonic_gpu(int *array, int j, int k) {
    int i, ixj;
    i = threadIdx.x + blockDim.x * blockIdx.x;
    ixj = i^j;
    if ((ixj)>i) {
        if ((i&k)==0) {
            if (array[i] > array[ixj]) {
                int temp = array[i];
                array[i] = array[ixj];
                array[ixj] = temp;
            }
        }
        if ((i&k)!=0) {
            if (array[i] < array[ixj]) {
                int temp = array[i];
                array[i] = array[ixj];
                array[ixj] = temp;
            }
        }
    } 
}

void bitonic_sort_gpu(int* array, int length) {
    int* gpu_array;
    cudaMalloc(&gpu_array, length);
    cudaMemcpy(gpu_array, array, length, cudaMemcpyHostToDevice);
    dim3 num_blocks(BLOCKS, 1);
	dim3 num_threads_in_block(THREADS, 1);
    for (int k = 2; k <= length; k <<= 1) {
		for (int j = k >> 1; j > 0; j >>= 1) {
			bitonic_gpu <<< num_blocks, num_threads_in_block >>> (gpu_array, j, k);
		}
	}
    cudaMemcpy(array, gpu_array, length, cudaMemcpyDeviceToHost);
    cudaFree(gpu_array);
}

int main(int argc, char **argv) {
    int length = NUM;
	int *array = (int*)malloc(sizeof(int) * length);
    int *array_gpu = (int*)malloc(sizeof(int) * length);

    srand(time(0));
	for (int i = 0; i < length; i++) {
		array[i] = rand() % 10000;
    }

    time_t start = clock();
    bitonic_sort(array, length);
    time_t stop = clock();
    double elapsed = ((double) (stop - start)) / CLOCKS_PER_SEC;

    if (!check(array, length)) {
		printf("Cpu check failed\n");
	} else {
          printf("Check successed\nElapsed time on cpu: %.3fs\n", elapsed);
    }

    memcpy(array_gpu, array, sizeof(int) * length);
    start = clock();
    bitonic_sort_gpu(array_gpu, length);
    stop = clock();
    elapsed = ((double) (stop - start)) / CLOCKS_PER_SEC;
    if (!check(array_gpu, length)) {
		printf("Gpu check failed\n");
	} else {
          printf("Check cuccessed\nElapsed time on cpu: %.3fs\n", elapsed);
    }
    free(array);
    return 0;
}
