#include <cstdlib>

/**
 * __cstr - converts a value to a string
 */
#define __cstr_t(x...)	#x
#define __cstr(x...)	__cstr_t(x)

/**
 * ARRAY_SIZE - get the number of elements in a visible array
 * @arr: the array whose size you want.
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define LATENCY_BENCHMARK(name) \
	{__cstr(name), &name ## _latency}


struct benchmark {
	const char *library;
	long (*measure)(size_t, int);
};

void fill_buffer_rand(unsigned char *buf, size_t buf_size);
long zlib_latency(size_t input_size, int level);
long libdeflate_latency(size_t input_size, int level);
long isal_latency(size_t input_size, int level);