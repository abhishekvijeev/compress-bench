#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>

#include <isa-l.h>
#include <libdeflate.h>
#include <zlib.h>

#include "compress-bench.h"


constexpr unsigned int ITERATIONS = 1000;
constexpr unsigned int LEVEL = 1;


long zlib_latency(size_t input_size, int level)
{
    unsigned char *input, *output;
    size_t output_size;
    struct timespec start_time, end_time;
    long latency = 0;
    int ret;

    output_size = compressBound(input_size);
    input = (unsigned char *) malloc(input_size * sizeof(unsigned char));
    output = (unsigned char *) malloc(output_size * sizeof(unsigned char));
    fill_buffer_rand(input, input_size);

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    ret = compress2(output, &output_size, input, input_size, level);
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    if (ret != Z_OK) {
        fprintf(stderr, "zlib compression failed with %d\n", ret);
        exit(0);
    }

    latency = end_time.tv_nsec - start_time.tv_nsec;
    free(output);
    free(input);

    return latency;
}

long libdeflate_latency(size_t input_size, int level)
{
    unsigned char *input, *output;
    size_t output_size;
    struct timespec start_time, end_time;
    long latency = 0;
    size_t ret;
    struct libdeflate_compressor *comp;
    struct libdeflate_decompressor *decomp;

    comp = libdeflate_alloc_compressor(level);
    if (comp == NULL) {
        printf("Unable to allocate compressor with level %d\n", level);
    }
    decomp = libdeflate_alloc_decompressor();
    if (decomp == NULL) {
        printf("Unable to allocate decompressor\n");
    }

    output_size = libdeflate_deflate_compress_bound(comp, input_size);
    // output_size = libdeflate_zlib_compress_bound(comp, input_size);
    // output_size = libdeflate_gzip_compress_bound(comp, input_size);

    input = (unsigned char *) malloc(input_size * sizeof(unsigned char));
    output = (unsigned char *) malloc(output_size * sizeof(unsigned char));
    // decomp_buf = (unsigned char *) malloc(input_size * sizeof(unsigned char));
    fill_buffer_rand(input, input_size);

    // printf("Input:\n");
    // for (int j = 0; j < input_size; j++) {
    //     printf("%d ", input[j]);
    // }
    // printf("\n\n");

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    ret = libdeflate_deflate_compress(comp, input, input_size, output, output_size);
    // ret = libdeflate_zlib_compress(comp, input, input_size, output, output_size);
    // ret = libdeflate_gzip_compress(comp, input, input_size, output, output_size);
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // printf("Compressed:\n");
    // for (int j = 0; j < output_size; j++) {
    //     printf("%d ", output[j]);
    // }
    // printf("\n\n");

    // libdeflate_deflate_decompress(decomp, output, output_size, decomp_buf, input_size, &actual);
    // printf("Decompressed:\n");
    // for (int j = 0; j < actual; j++) {
    //     printf("%d ", decomp_buf[j]);
    // }
    // printf("\n\n");

    latency = end_time.tv_nsec - start_time.tv_nsec;
    free(output);
    free(input);
    libdeflate_free_compressor(comp);
    libdeflate_free_decompressor(decomp);

    return latency;
}

long isal_latency(size_t input_size, int level)
{
    return 3.0;
}

static const struct benchmark latency_benchmarks[] = {
	LATENCY_BENCHMARK(zlib),
	LATENCY_BENCHMARK(libdeflate),
    LATENCY_BENCHMARK(isal),
};

int main()
{
    srand(time(NULL));

    size_t buf_sizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024,
    2048, 4096, 8192, 16384, 32768};

    for (size_t i = 0; i < ARRAY_SIZE(buf_sizes); i++) {
        size_t buf_size = buf_sizes[i];

        std::unordered_map<std::string, long> m_library_to_latency;
        for (size_t j = 0; j < ARRAY_SIZE(latency_benchmarks); j++) {

            for (unsigned int k = 0; k < ITERATIONS; k++) {
                m_library_to_latency[latency_benchmarks[j].library] += latency_benchmarks[j].measure(buf_size, LEVEL);
            }
        }

        printf("Buffer Size %lu\n", buf_size);
        for (auto& it : m_library_to_latency) {
            const char *lib_name = it.first.c_str();
            // double total_latency_us = it.second / (1000;
            long double d = 1000.0 * ITERATIONS;
            std::cout << it.first << "->" << it.second << std::endl;
            // double 
            // printf("%s -> %lf us\n", lib_name, total_latency_us );
        }
        printf("\n\n");
    }
    
    return 0;
}

void fill_buffer_rand(unsigned char *buf, size_t buf_size)
{
    if (buf == NULL) {
        fprintf(stderr, "Passed buffer is empty");
        exit(0);
    }

    for (int i = 0; i < buf_size; i++) {
        buf[i] = rand();
    }
}