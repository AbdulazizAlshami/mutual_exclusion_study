#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdalign.h>

#define MAX_THREADS 128

typedef enum {
    MODE_MUTEX = 0,
    MODE_SPINLOCK,
    MODE_FAA,
    MODE_PACKED,
    MODE_PADDED
} Strategy;

// typedef struct {
//     uint64_t val;
// } CounterPacked;

// typedef struct {
//     alignas(64) uint64_t val;
//     uint8_t pad[56];
// } CounterPadded;

typedef struct {
    volatile uint64_t val;
} CounterPacked;

typedef struct {
    alignas(64) volatile uint64_t val;
    uint8_t pad[56];
} CounterPadded;

static uint64_t g_shared_counter = 0;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_spinlock_t g_spin;
static CounterPacked g_packed[MAX_THREADS];
static CounterPadded g_padded[MAX_THREADS];

static pthread_barrier_t g_barrier_start;
static pthread_barrier_t g_barrier_end;

static Strategy g_mode;
static uint64_t g_increments_per_thread;

void* worker_func(void* arg) {
    long tid = (long)arg;

    pthread_barrier_wait(&g_barrier_start);

    switch (g_mode) {
        case MODE_MUTEX:
            for (uint64_t i = 0; i < g_increments_per_thread; i++) {
                pthread_mutex_lock(&g_mutex);
                g_shared_counter++;
                pthread_mutex_unlock(&g_mutex);
            }
            break;

        case MODE_SPINLOCK:
            for (uint64_t i = 0; i < g_increments_per_thread; i++) {
                pthread_spin_lock(&g_spin);
                g_shared_counter++;
                pthread_spin_unlock(&g_spin);
            }
            break;

        case MODE_FAA:
            for (uint64_t i = 0; i < g_increments_per_thread; i++) {
                __atomic_fetch_add(&g_shared_counter, 1, __ATOMIC_SEQ_CST);
            }
            break;

        case MODE_PACKED:
            for (uint64_t i = 0; i < g_increments_per_thread; i++) {
                g_packed[tid].val++;
            }
            break;

        case MODE_PADDED:
            for (uint64_t i = 0; i < g_increments_per_thread; i++) {
                g_padded[tid].val++;
            }
            break;
    }

    pthread_barrier_wait(&g_barrier_end);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <strategy: 0-4> <threads> <total_increments>\n", argv[0]);
        return 1;
    }

    g_mode = (Strategy)atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    uint64_t total_increments = strtoull(argv[3], NULL, 10);
    g_increments_per_thread = total_increments / num_threads;

    pthread_spin_init(&g_spin, PTHREAD_PROCESS_PRIVATE);
    pthread_barrier_init(&g_barrier_start, NULL, num_threads + 1);
    pthread_barrier_init(&g_barrier_end, NULL, num_threads + 1);

    pthread_t threads[MAX_THREADS];
    for (long i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_func, (void*)i);
    }

    struct timespec start, end;

    pthread_barrier_wait(&g_barrier_start);
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    pthread_barrier_wait(&g_barrier_end);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double elapsed_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double mops = ((double)total_increments / elapsed_sec) / 1e6;

    printf("%d,%d,%lu,%.6f,%.2f\n", g_mode, num_threads, total_increments, elapsed_sec, mops);

    pthread_spin_destroy(&g_spin);
    pthread_barrier_destroy(&g_barrier_start);
    pthread_barrier_destroy(&g_barrier_end);
    return 0;
}
