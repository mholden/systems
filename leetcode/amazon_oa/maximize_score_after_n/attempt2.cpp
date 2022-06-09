#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <numeric>
#include <unordered_map>
#include <bitset>
#include <string.h>
#include <inttypes.h>
#include <vector>
#include <iostream>

using namespace std;

static int **gcds;
static unordered_map<int, int> **dp;

static int _max_score(int *nums, int nsize, int start_mult, int& bm) {
    int max, this_sum, sub_max;
    
    //printf("_max_score: start: start mult %d bm 0x%04X\n", start_mult, bm);
    
    auto dpval = dp[start_mult-1]->find(bm);
    if (dpval != dp[start_mult-1]->end()) {
        max = dpval->second;
        goto out;
    }
    
    max = 0;
    for (int i = 0; i < nsize; i++) {
        if (bm & (1 << i))
            continue;
        bm |= (1 << i);
        for (int j = i + 1; j < nsize; j++) {
            if (bm & (1 << j))
                continue;
            bm |= (1 << j);
            this_sum = start_mult * gcds[i][j];
            sub_max = _max_score(nums, nsize, start_mult + 1, bm);
            if (sub_max + this_sum > max)
                max = sub_max + this_sum;
            bm &= ~(1 << j);
        }
        bm &= ~(1 << i);
    }
    
    dp[start_mult-1]->insert({bm, max});
    
out:
    return max;
}

int maxScore(int* nums, int numsSize) {
    int bm; // bitmap
    int max;
    
    //printf("maxScore: start\n");
    
    assert(numsSize <= sizeof(int) * 8);
    
    assert(gcds = (int **)malloc(sizeof(int *) * numsSize));
    for (int i = 0; i < numsSize; i++) {
        assert(gcds[i] = (int *)malloc(sizeof(int) * numsSize));
        memset(gcds[i], 0, sizeof(int) * numsSize);
    }
    
    assert(dp = (unordered_map<int, int> **)malloc(sizeof(unordered_map<int, int> *) * numsSize));
    for (int i = 0; i < numsSize; i++)
        dp[i] = new unordered_map<int, int>;
    
    // pre-compute gcds
    for (int i = 0; i < numsSize; i++) {
        for (int j = i + 1; j < numsSize; j++)
            gcds[i][j] = gcd(nums[i], nums[j]);
    }
    
    bm = 0;
    max = _max_score(nums, numsSize, 1, bm);
    
    for (int i = 0; i < numsSize; i++)
        free(gcds[i]);
    free(gcds);
    for (int i = 0; i < numsSize; i++)
        delete dp[i];
    free(dp);
    
    return max;
}

static double diff_timespec(struct timespec tend, struct timespec tstart) {
    return (tend.tv_sec - tstart.tv_sec) + ((tend.tv_nsec - tstart.tv_nsec) / 1e9);
}

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
