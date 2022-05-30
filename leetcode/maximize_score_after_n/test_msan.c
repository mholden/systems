#include <stdio.h>
#include <assert.h>
#include <time.h>

int maxScore(int* nums, int numsSize);

static double diff_timespec(struct timespec tend, struct timespec tstart) {
    return (tend.tv_sec - tstart.tv_sec) + ((tend.tv_nsec - tstart.tv_nsec) / 1e9);
}

//
// XXX: this test case fails. it times out.
// you need to use dynamic programming to
// remember all of the gcds and sub-solutions
// you've worked through so that you're not
// doing unnecessary work more than once
//
static void test_msan_specific_case4(void) {
    int nums[8];
    struct timespec tstart, tend;
    double measured_time;
    
    //[370435,481435,953948,282360,691237,574616,638525,764832]
    
    nums[0] = 370435;
    nums[1] = 481435;
    nums[2] = 953948;
    nums[3] = 282360;
    nums[4] = 691237;
    nums[5] = 574616;
    nums[6] = 638525;
    nums[7] = 764832;
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    assert(maxScore(nums, 8)/* == 14*/);
    clock_gettime(CLOCK_MONOTONIC, &tend);
    
    measured_time = diff_timespec(tend, tstart);
    
    printf("test_msan_specific_case4 time: %.3fs\n", measured_time);
    
}

static void test_msan_specific_case3(void) {
    int nums[6];
    
    nums[0] = 1;
    nums[1] = 2;
    nums[2] = 3;
    nums[3] = 4;
    nums[4] = 5;
    nums[5] = 6;
    assert(maxScore(nums, 6) == 14);
}

static void test_msan_specific_case2(void) {
    int nums[4];
    
    nums[0] = 3;
    nums[1] = 4;
    nums[2] = 6;
    nums[3] = 8;
    assert(maxScore(nums, 4) == 11);
}

static void test_msan_specific_case1(void) {
    int nums[2];
    
    nums[0] = 1;
    nums[1] = 2;
    assert(maxScore(nums, 2) == 1);
}

static void test_msan(void) {
    test_msan_specific_case1();
    test_msan_specific_case2();
    test_msan_specific_case3();
    test_msan_specific_case4();
}

int main(int argc, const char *argv[]) {
    test_msan();
    return 0;
}
