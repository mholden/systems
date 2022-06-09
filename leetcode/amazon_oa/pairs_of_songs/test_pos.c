#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

int numPairsDivisibleBy60(int* time, int timeSize);

static double diff_timespec(struct timespec tend, struct timespec tstart) {
    return (tend.tv_sec - tstart.tv_sec) + ((tend.tv_nsec - tstart.tv_nsec) / 1e9);
}

static void test_prog1_rand_case1(void) {
    int *t, n = 60000, result;
    struct timespec tstart, tend;
    double measured_time;
    
    srand(time(NULL));
    
    assert(t = malloc(n * sizeof(int)));
    for (int i = 0; i < n; i++)
        t[i] = rand();
    
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    result = numPairsDivisibleBy60(t, n);
    clock_gettime(CLOCK_MONOTONIC, &tend);
    measured_time = diff_timespec(tend, tstart);
    
    printf("test_prog1_rand_case1: %d time: %.3fs\n", result, measured_time);
    
}

static void test_prog1_specific_case1(void) {
    int time[5];
    
    time[0] = 30;
    time[1] = 20;
    time[2] = 150;
    time[3] = 100;
    time[4] = 40;
    assert(numPairsDivisibleBy60(time, 5) == 3);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
    test_prog1_rand_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
