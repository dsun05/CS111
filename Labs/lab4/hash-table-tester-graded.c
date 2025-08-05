#include "hash-table-base.h"
#include "hash-table-v1.h"
#include "hash-table-v2.h"

#include <argp.h>
#include <locale.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>

#define BYTES_PER_STRING 8
#define NUM_RUNS 3

struct arguments {
	uint32_t threads;
	uint32_t size;
};

static struct argp_option options[] = {
	{ "threads", 't', "NUM", 0, "Number of threads." },
	{ "size", 's', "NUM", 0, "Size per thread." },
	{ 0 }
};

static uint32_t parse_uint32_t(const char *string)
{
	uint32_t current = 0;
	uint8_t i = 0;
	while (true) {
		char c = string[i];
		if (c == 0) {
			break;
		}
		if (i == 10) {
			exit(EINVAL);
		}
		if (c < 0x30 || c > 0x39) {
			exit(EINVAL);
		}
		uint8_t digit = (c - 0x30);
		if (i == 9) {
			if (current > 429496729) {
				exit(EINVAL);
			} else if (current == 429496729 && digit > 5) {
				exit(EINVAL);
			}
		}
		current = current * 10 + digit;
		++i;
	}
	return current;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	switch (key) {
	case 't':
		arguments->threads = parse_uint32_t(arg);
		break;
	case 's':
		arguments->size = parse_uint32_t(arg);
		break;
	}
	return 0;
}

static struct arguments arguments;
static char *data;

static size_t get_global_index(uint32_t thread, uint32_t index)
{
	return thread * arguments.size + index;
}

static char *get_string(size_t global_index)
{
	return data + (global_index * BYTES_PER_STRING);
}

static unsigned long usec_diff(struct timeval *a, struct timeval *b)
{
	unsigned long usec;
	usec = (b->tv_sec - a->tv_sec) * 1000000;
	usec += b->tv_usec - a->tv_usec;
	return usec;
}

static struct hash_table_v1 *hash_table_v1;

void *run_v1(void *arg)
{
	uint32_t thread = (uintptr_t)arg;
	for (uint32_t j = 0; j < arguments.size; ++j) {
		size_t global_index = get_global_index(thread, j);
		char *string = get_string(global_index);
		hash_table_v1_add_entry(hash_table_v1, string, global_index);
	}
	return NULL;
}

static struct hash_table_v2 *hash_table_v2;

void *run_v2(void *arg)
{
	uint32_t thread = (uintptr_t)arg;
	for (uint32_t j = 0; j < arguments.size; ++j) {
		size_t global_index = get_global_index(thread, j);
		char *string = get_string(global_index);
		hash_table_v2_add_entry(hash_table_v2, string, global_index);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	arguments.threads = 4;
	arguments.size = 25000;

	static struct argp argp = { options, parse_opt };
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	setlocale(LC_ALL, "en_US.UTF-8");

	data = calloc(arguments.threads * arguments.size, BYTES_PER_STRING);
	if (!data) {
		perror("Failed to allocate data");
		return 1;
	}

	struct timeval start, end;
	gettimeofday(&start, NULL);
	srand(42);
	for (uint32_t i = 0; i < arguments.threads; ++i) {
		for (uint32_t j = 0; j < arguments.size; ++j) {
			size_t global_index = get_global_index(i, j);
			char *string = get_string(global_index);
			for (uint32_t k = 0; k < (BYTES_PER_STRING - 1); ++k) {
				int r = rand() % 52;
				string[k] = (r < 26) ? (r + 0x41) : (r + 0x47);
			}
			string[BYTES_PER_STRING - 1] = 0;
		}
	}
	gettimeofday(&end, NULL);
	printf("Data generation: %'lu usec\n", usec_diff(&start, &end));

	unsigned long base_timings[NUM_RUNS], v1_timings[NUM_RUNS], v2_timings[NUM_RUNS];
	size_t base_missing[NUM_RUNS], v1_missing[NUM_RUNS], v2_missing[NUM_RUNS];

	pthread_t *threads = calloc(arguments.threads, sizeof(pthread_t));
	if (!threads) {
		perror("Failed to allocate threads");
		free(data);
		return 1;
	}

	for (int run = 0; run < NUM_RUNS; ++run) {
		printf("\n--- Run %d/%d ---\n", run + 1, NUM_RUNS);

		struct hash_table_base *hash_table_base = hash_table_base_create();
		gettimeofday(&start, NULL);
		for (uint32_t i = 0; i < arguments.threads; ++i) {
			for (uint32_t j = 0; j < arguments.size; ++j) {
				size_t global_index = get_global_index(i, j);
				char *string = get_string(global_index);
				hash_table_base_add_entry(hash_table_base, string, global_index);
			}
		}
		gettimeofday(&end, NULL);
		base_timings[run] = usec_diff(&start, &end);
		printf("Hash table base: %'lu usec\n", base_timings[run]);

		base_missing[run] = 0;
		for (uint32_t i = 0; i < arguments.threads; ++i) {
			for (uint32_t j = 0; j < arguments.size; ++j) {
				size_t global_index = get_global_index(i, j);
				char *string = get_string(global_index);
				if (!hash_table_base_contains(hash_table_base, string)) {
					base_missing[run]++;
				}
			}
		}
		printf("  - %'zu missing\n", base_missing[run]);
		hash_table_base_destroy(hash_table_base);

		hash_table_v1 = hash_table_v1_create();
		gettimeofday(&start, NULL);
		for (uintptr_t i = 0; i < arguments.threads; ++i) {
			pthread_create(&threads[i], NULL, run_v1, (void *)i);
		}
		for (uintptr_t i = 0; i < arguments.threads; ++i) {
			pthread_join(threads[i], NULL);
		}
		gettimeofday(&end, NULL);
		v1_timings[run] = usec_diff(&start, &end);
		printf("Hash table v1: %'lu usec\n", v1_timings[run]);

		v1_missing[run] = 0;
		for (uint32_t i = 0; i < arguments.threads; ++i) {
			for (uint32_t j = 0; j < arguments.size; ++j) {
				size_t global_index = get_global_index(i, j);
				char *string = get_string(global_index);
				if (!hash_table_v1_contains(hash_table_v1, string)) {
					v1_missing[run]++;
				}
			}
		}
		printf("  - %'zu missing\n", v1_missing[run]);
		hash_table_v1_destroy(hash_table_v1);

		hash_table_v2 = hash_table_v2_create();
		gettimeofday(&start, NULL);
		for (uintptr_t i = 0; i < arguments.threads; ++i) {
			pthread_create(&threads[i], NULL, run_v2, (void *)i);
		}
		for (uintptr_t i = 0; i < arguments.threads; ++i) {
			pthread_join(threads[i], NULL);
		}
		gettimeofday(&end, NULL);
		v2_timings[run] = usec_diff(&start, &end);
		printf("Hash table v2: %'lu usec\n", v2_timings[run]);

		v2_missing[run] = 0;
		for (uint32_t i = 0; i < arguments.threads; ++i) {
			for (uint32_t j = 0; j < arguments.size; ++j) {
				size_t global_index = get_global_index(i, j);
				char *string = get_string(global_index);
				if (!hash_table_v2_contains(hash_table_v2, string)) {
					v2_missing[run]++;
				}
			}
		}
		printf("  - %'zu missing\n", v2_missing[run]);
		hash_table_v2_destroy(hash_table_v2);
	}

	printf("\n--- GRADING SUMMARY ---\n");

	bool correctness_ok = true;
	for (int i = 0; i < NUM_RUNS; ++i) {
		if (base_missing[i] > 0 || v1_missing[i] > 0 || v2_missing[i] > 0) {
			correctness_ok = false;
			printf("CORRECTNESS FAILED: Run %d missed %'zu (base), %'zu (v1), %'zu (v2) elements.\n",
			       i + 1, base_missing[i], v1_missing[i], v2_missing[i]);
		}
	}

	if (!correctness_ok) {
		printf("\nOverall Result: No credit awarded due to correctness failure.\n");
		free(threads);
		free(data);
		return 1;
	}

	printf("CORRECTNESS PASSED: All elements accounted for in all runs.\n\n");

	unsigned long best_base_time = ULONG_MAX, best_v1_time = ULONG_MAX, best_v2_time = ULONG_MAX;
	for (int i = 0; i < NUM_RUNS; ++i) {
		if (base_timings[i] < best_base_time) best_base_time = base_timings[i];
		if (v1_timings[i] < best_v1_time) best_v1_time = v1_timings[i];
		if (v2_timings[i] < best_v2_time) best_v2_time = v2_timings[i];
	}

	printf("Best Times: Base=%'lu usec, V1=%'lu usec, V2=%'lu usec\n",
	       best_base_time, best_v1_time, best_v2_time);
	
	printf("\n--- V1 Performance Grade ---\n");
	if (best_v1_time > best_base_time) {
		printf("PASSED: V1 was slower than Base, as expected.\n");
	} else {
		printf("FAILED: V1 was not slower than Base.\n");
	}

	printf("\n--- V2 Performance Grade ---\n");
	long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cores <= 1) {
		printf("Warning: Only one core detected. Meaningful performance scaling tests are not possible.\n");
		num_cores = 2;
	}

	double high_perf_target = (double)best_base_time / (num_cores - 1);
	double weak_perf_target = (double)best_base_time / (num_cores / 2.0);

	printf("Number of cores detected: %ld\n", num_cores);
	printf("High performance target (Test 1: base / (cores - 1)): <= %'.0f usec\n", high_perf_target);
	printf("Weak performance target (Test 2: base / (cores / 2)): <= %'.0f usec\n", weak_perf_target);

	if (best_v2_time <= high_perf_target) {
		printf("\nResult: PASSED Test 1 (High Performance Criteria Met)\n");
	} else if (best_v2_time <= weak_perf_target) {
		printf("\nResult: PASSED Test 2 (Weak Performance Criteria Met)\n");
	} else {
		printf("\nResult: FAILED Test 1 & 2. Would be evaluated against Test 3 (Low number of elements).\n");
	}

	free(threads);
	free(data);

	return 0;
}
