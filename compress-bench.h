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
	int64_t (*measure)(size_t, int);
};

int level_buf_sizes[4] = {
#ifdef ISAL_DEF_LVL0_DEFAULT
	ISAL_DEF_LVL0_DEFAULT,
#else
	0,
#endif

#ifdef ISAL_DEF_LVL1_DEFAULT
	ISAL_DEF_LVL1_DEFAULT,
#else
	0,
#endif

#ifdef ISAL_DEF_LVL2_DEFAULT
	ISAL_DEF_LVL2_DEFAULT,
#else
	0,
#endif

#ifdef ISAL_DEF_LVL3_DEFAULT
	ISAL_DEF_LVL3_DEFAULT,
#else
	0,
#endif
};

void fill_buffer_rand(unsigned char *buf, size_t buf_size);
int64_t zlib_latency(size_t input_size, int level);
int64_t libdeflate_latency(size_t input_size, int level);
int64_t isal_latency(size_t input_size, int level);