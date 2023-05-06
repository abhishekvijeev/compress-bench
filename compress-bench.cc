#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

#include <isa-l.h>
#include <libdeflate.h>
#include <zlib.h>

#include "compress-bench.h"

constexpr unsigned int ITERATIONS = 1000;
constexpr unsigned int LEVEL = 1;
constexpr unsigned int WINDOW_SIZE = 10;
constexpr double NANO_TO_MICRO = 0.001;

int64_t zlib_latency(size_t input_size, int level)
{
    unsigned char *input, *output;
    size_t output_size;
    long latency = 0;
    int ret;

    output_size = compressBound(input_size);
    input = (unsigned char *) malloc(input_size * sizeof(unsigned char));
    if (input == NULL) {
        fprintf(stderr, "zlib_latency: could not allocate input buffer\n");
        exit(0);
    }
    output = (unsigned char *) malloc(output_size * sizeof(unsigned char));
    if (output == NULL) {
        fprintf(stderr, "zlib_latency: could not allocate output buffer\n");
        exit(0);
    }
    fill_buffer_rand(input, input_size);

    auto start_time = std::chrono::steady_clock::now();
    ret = compress2(output, &output_size, input, input_size, level);
    auto end_time = std::chrono::steady_clock::now();
    auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    if (ret != Z_OK) {
        fprintf(stderr, "zlib compression failed with %d\n", ret);
        exit(0);
    }

    free(output);
    free(input);

    return latency_ns.count();
}

int64_t libdeflate_latency(size_t input_size, int level)
{
    unsigned char *input, *output;
    size_t output_size;
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

    input = (unsigned char *) malloc(input_size * sizeof(unsigned char));
    if (input == NULL) {
        fprintf(stderr, "libdeflate_latency: could not allocate input buffer\n");
        exit(0);
    }
    output = (unsigned char *) malloc(output_size * sizeof(unsigned char));
    if (output == NULL) {
        fprintf(stderr, "libdeflate_latency: could not allocate output buffer\n");
        exit(0);
    }
    fill_buffer_rand(input, input_size);

    auto start_time = std::chrono::steady_clock::now();
    ret = libdeflate_deflate_compress(comp, input, input_size, output, output_size);
    auto end_time = std::chrono::steady_clock::now();
    auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    if (ret == 0) {
        fprintf(stderr, "libdeflate compression failed\n");
        exit(0);
    }

    free(output);
    free(input);
    libdeflate_free_compressor(comp);
    libdeflate_free_decompressor(decomp);

    return latency_ns.count();
}

int64_t isal_latency(size_t input_size, int level)
{
    unsigned char *input, *output, *level_buf;
    size_t output_size, level_buf_size;
    struct isal_zstream stream;
    int ret;

    output_size = ISAL_DEF_MAX_HDR_SIZE + input_size;
    level_buf_size = level_buf_sizes[LEVEL];

    input = (unsigned char *)malloc(input_size * sizeof (unsigned char));
    if (input == NULL) {
        fprintf(stderr, "Can't allocate memory for input buffer\n");
        exit(0);
    }

    output = (unsigned char *)malloc(output_size * sizeof (unsigned char));
    if (output == NULL) {
        fprintf(stderr, "Can't allocate memory for output buffer\n");
        exit(0);
    }

    level_buf = (uint8_t *)malloc(level_buf_size * sizeof (uint8_t));
    if (level_buf == NULL) {
        fprintf(stderr, "Can't allocate memory for level buffer\n");
        exit(0);
    }

    fill_buffer_rand(input, input_size);

    isal_deflate_init(&stream);
    stream.end_of_stream = 1;	/* Do the entire file at once */
    stream.flush = NO_FLUSH;
    stream.next_in = input;
    stream.avail_in = input_size;
    stream.next_out = output;
    stream.avail_out = output_size;
    stream.level = LEVEL;
    stream.level_buf = level_buf;
    stream.level_buf_size = level_buf_size;
    stream.hist_bits = WINDOW_SIZE;

    auto start_time = std::chrono::steady_clock::now();
    ret = isal_deflate_stateless(&stream);
    auto end_time = std::chrono::steady_clock::now();
    auto latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    if (ret != COMP_OK) {
        fprintf(stderr, "ISA-L compress failed with %d\n", ret);
        exit(0);
    }

    free(input);
    free(output);
    free(level_buf);

    return latency_ns.count();
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

        printf("%lu bytes:\n", buf_size);
        for (auto& it : m_library_to_latency) {
            std::cout << it.first << "->" << (it.second * NANO_TO_MICRO) / ITERATIONS << " us" << std::endl;
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