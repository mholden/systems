#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>

using namespace std;

static int ***dp;

static void _subarray_ranges(int *a, int asz, int ind, long long *sum) {
    long long this_sum = 0;
    
    if (ind >= asz)
        goto out;
    
    _subarray_ranges(a, asz, ind + 1, &this_sum);
    
    dp[ind][ind][0] = a[ind];
    dp[ind][ind][1] = a[ind];
    
    for (int i = ind + 1; i < asz; i++) {
        // set min
        if (a[ind] < dp[ind + 1][i][0])
            dp[ind][i][0] = a[ind];
        else
            dp[ind][i][0] = dp[ind + 1][i][0];
        // set max
        if (a[ind] > dp[ind + 1][i][1])
            dp[ind][i][1] = a[ind];
        else
            dp[ind][i][1] = dp[ind + 1][i][1];
        // adjust sum
        this_sum += dp[ind][i][1] - dp[ind][i][0];
    }
    
    *sum = this_sum;
    
out:
    return;
}

long long _subArrayRanges(int* nums, int numsSize){
    long long sum;
    
    assert(dp = (int ***)malloc(sizeof(int *) * numsSize));
    for (int i = 0; i < numsSize; i++) {
        assert(dp[i] = (int **)malloc(sizeof(int *) * numsSize));
        for (int j = 0; j < numsSize; j++) {
            assert(dp[i][j] = (int *)malloc(sizeof(int) * 2));
            memset(dp[i][j], 0, sizeof(int) * 2);
        }
    }
    
    sum = 0;
    _subarray_ranges(nums, numsSize, 0, &sum);
    
    //dump_dp(dp);
    
    // TODO: free dp
    
    return sum;
}

// XXX: this is the best and fastest solution
// solution above in C doesn't even pass on leetcode
// but same solution (alt) in C++ does pass..
long long subArrayRanges(vector<int>& nums) {
    long long sum;
    int _max, _min;

    sum = 0;
    for (int i = 0; i < nums.size(); i++) {
        _max = _min = nums[i];
        for (int j = i; j < nums.size(); j++) {
            _min = min(nums[j], _min);
            _max = max(nums[j], _max);
            sum += _max - _min;
        }
    }

    return sum;
}

static void test_prog1_specific_case1(void) {
    int n[3] = {1, 2, 3};
    vector<int> v = {1, 2, 3};
    assert(subArrayRanges(v) == 4);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
