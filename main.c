{
  "nbformat": 4,
  "nbformat_minor": 0,
  "metadata": {
    "colab": {
      "name": "Untitled0.ipynb",
      "provenance": [],
      "collapsed_sections": []
    },
    "kernelspec": {
      "name": "python3",
      "display_name": "Python 3"
    },
    "accelerator": "GPU"
  },
  "cells": [
    {
      "cell_type": "code",
      "metadata": {
        "id": "r2qSNe1v9t8R",
        "colab_type": "code",
        "outputId": "f34fb4de-9cc3-4633-e7f1-f3c8f7afc676",
        "colab": {
          "base_uri": "https://localhost:8080/",
          "height": 107
        }
      },
      "source": [
        "%%cu\n",
        "#include \"cuda_runtime.h\"\n",
        "#include \"device_launch_parameters.h\"\n",
        "#include <stdio.h>\n",
        "#include <stdlib.h>\n",
        "#include <cmath>\n",
        "#include <ctime>\n",
        "\n",
        "#define THREADS 256\n",
        "#define BLOCKS 32768\n",
        "#define NUM THREADS * BLOCKS\n",
        "\n",
        "bool check(int *array, int length) {\n",
        "\tbool result = true;\n",
        "\tfor (int i = 0; i < length - 1; i++) {\n",
        "\t\tif (array[i] > array[i + 1]) {\n",
        "\t\t    return false;\n",
        "\t\t}\n",
        "\t}\n",
        "\treturn result;\n",
        "}\n",
        "\n",
        "void bitonic(int* array, int length) {\n",
        "\tint K = log2(length);\n",
        "\tint d = 1 << K;\n",
        "    K--;\n",
        "\tfor (int n = 0; n < d >> 1; n++) {\n",
        "        if (array[n] > array[d - n - 1]) {\n",
        "            int temp = array[n];\n",
        "            array[n] = array[d - n - 1];\n",
        "            array[d - n - 1] = temp;\n",
        "        }\n",
        "    }\n",
        "\tfor (int k = K; k > 0; k--) {\n",
        "\t\td = 1 << k;\n",
        "\t\tfor (int m = 0; m < length; m += d) {\n",
        "\t\t    for (int n = 0; n < d >> 1; n++) {\n",
        "                if (array[m + n] > array[m + (d >> 1) + n]) {\n",
        "                    int temp = array[m + n];\n",
        "                    array[m + n] = array[m + (d >> 1) + n];\n",
        "                    array[m + (d >> 1) + n] = temp;\n",
        "                }\n",
        "            }\n",
        "        }\n",
        "\t}\n",
        "}\n",
        "\n",
        "void bitonic_sort(int* array, int length) {\n",
        "    int* arr = (int*)malloc(sizeof(int) * length);\n",
        "\tfor (int n = 0; n < length; n++)\n",
        "\t\tarr[n] = array[n];\n",
        "\tint K = log2(length);\n",
        "\tfor (int k = 1; k <= K; k++) {\n",
        "\t\tfor (int n = 0; n < length; n += 1 << k) {\n",
        "            int* arr_ptr = &arr[n];\n",
        "\t\t\tbitonic(arr_ptr, 1 << k);\n",
        "\t\t}\n",
        "\t}\n",
        "\tfor (int n = 0; n < length; n++) {\n",
        "\t\tarray[n] = arr[n];\n",
        "\t}\n",
        "\tfree(arr);\n",
        "}\n",
        "\n",
        "__global__ void bitonic_gpu(int *array, int j, int k) {\n",
        "    int i, ixj;\n",
        "    i = threadIdx.x + blockDim.x * blockIdx.x;\n",
        "    ixj = i^j;\n",
        "    if ((ixj)>i) {\n",
        "        if ((i&k)==0) {\n",
        "            if (array[i] > array[ixj]) {\n",
        "                int temp = array[i];\n",
        "                array[i] = array[ixj];\n",
        "                array[ixj] = temp;\n",
        "            }\n",
        "        }\n",
        "        if ((i&k)!=0) {\n",
        "            if (array[i] < array[ixj]) {\n",
        "                int temp = array[i];\n",
        "                array[i] = array[ixj];\n",
        "                array[ixj] = temp;\n",
        "            }\n",
        "        }\n",
        "    } \n",
        "}\n",
        "\n",
        "void bitonic_sort_gpu(int* array, int length) {\n",
        "    int* gpu_array;\n",
        "    cudaMalloc(&gpu_array, length);\n",
        "    cudaMemcpy(gpu_array, array, length, cudaMemcpyHostToDevice);\n",
        "    dim3 num_blocks(BLOCKS, 1);\n",
        "\tdim3 num_threads_in_block(THREADS, 1);\n",
        "    for (int k = 2; k <= length; k <<= 1) {\n",
        "\t\tfor (int j = k >> 1; j > 0; j >>= 1) {\n",
        "\t\t\tbitonic_gpu <<< num_blocks, num_threads_in_block >>> (gpu_array, j, k);\n",
        "\t\t}\n",
        "\t}\n",
        "    cudaMemcpy(array, gpu_array, length, cudaMemcpyDeviceToHost);\n",
        "    cudaFree(gpu_array);\n",
        "}\n",
        "\n",
        "int main(int argc, char **argv) {\n",
        "    int length = NUM;\n",
        "\tint *array = (int*)malloc(sizeof(int) * length);\n",
        "    int *array_gpu = (int*)malloc(sizeof(int) * length);\n",
        "\n",
        "    srand(time(0));\n",
        "\tfor (int i = 0; i < length; i++) {\n",
        "\t\tarray[i] = rand() % 10000;\n",
        "    }\n",
        "\n",
        "    time_t start = clock();\n",
        "    bitonic_sort(array, length);\n",
        "    time_t stop = clock();\n",
        "    double elapsed = ((double) (stop - start)) / CLOCKS_PER_SEC;\n",
        "\n",
        "    if (!check(array, length)) {\n",
        "\t\tprintf(\"Cpu check failed\\n\");\n",
        "\t} else {\n",
        "          printf(\"Check successed\\nElapsed time on cpu: %.3fs\\n\", elapsed);\n",
        "    }\n",
        "\n",
        "    memcpy(array_gpu, array, sizeof(int) * length);\n",
        "    start = clock();\n",
        "    bitonic_sort_gpu(array_gpu, length);\n",
        "    stop = clock();\n",
        "    elapsed = ((double) (stop - start)) / CLOCKS_PER_SEC;\n",
        "    if (!check(array_gpu, length)) {\n",
        "\t\tprintf(\"Gpu check failed\\n\");\n",
        "\t} else {\n",
        "          printf(\"Check cuccessed\\nElapsed time on cpu: %.3fs\\n\", elapsed);\n",
        "    }\n",
        "    free(array);\n",
        "    return 0;\n",
        "}"
      ],
      "execution_count": 61,
      "outputs": [
        {
          "output_type": "stream",
          "text": [
            "Check successed\n",
            "Elapsed time on cpu: 7.004s\n",
            "Check cuccessed\n",
            "Elapsed time on cpu: 0.102s\n",
            "\n"
          ],
          "name": "stdout"
        }
      ]
    },
    {
      "cell_type": "code",
      "metadata": {
        "id": "qMRzjo89tCsW",
        "colab_type": "code",
        "colab": {}
      },
      "source": [
        "!ls \"src/\""
      ],
      "execution_count": 0,
      "outputs": []
    },
    {
      "cell_type": "code",
      "metadata": {
        "id": "vDRsy4FIRuQC",
        "colab_type": "code",
        "outputId": "ce3e6e5c-55fb-489d-9eef-3fe6a110eec4",
        "colab": {
          "base_uri": "https://localhost:8080/",
          "height": 53
        }
      },
      "source": [
        "%load_ext nvcc_plugin"
      ],
      "execution_count": 58,
      "outputs": [
        {
          "output_type": "stream",
          "text": [
            "The nvcc_plugin extension is already loaded. To reload it, use:\n",
            "  %reload_ext nvcc_plugin\n"
          ],
          "name": "stdout"
        }
      ]
    }
  ]
}