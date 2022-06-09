#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>

using namespace std;

static vector<vector<pair<int, int>>> dp;

static void _subarray_ranges(vector<int>& a, int ind, long long *sum) {
    long long this_sum = 0;
    
    if (ind >= a.size())
        goto out;
    
    _subarray_ranges(a, ind + 1, &this_sum);
    
    dp[ind][ind].first = a[ind];
    dp[ind][ind].second = a[ind];
    
    for (int i = ind + 1; i < a.size(); i++) {
        // set min
        dp[ind][i].first = min(a[ind], dp[ind + 1][i].first);
        // set max
        dp[ind][i].second = max(a[ind], dp[ind + 1][i].second);
        // adjust sum
        this_sum += dp[ind][i].second - dp[ind][i].first;
    }
    
    *sum = this_sum;
    
out:
    return;
}

long long subArrayRanges(vector<int>& a){
    vector<vector<pair<int, int>>> v(a.size(), vector<pair<int, int>>(a.size()));
    long long sum;
    
    dp = v;
    sum = 0;
    _subarray_ranges(a, 0, &sum);
    
    //dump_dp(dp);
    
    // TODO: free dp
    
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
