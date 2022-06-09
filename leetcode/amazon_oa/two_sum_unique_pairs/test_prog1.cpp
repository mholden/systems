#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <unordered_set>
#include <iostream>

using namespace std;

int two_sum_unique_pairs(vector<int>& nums, int target) {
    unordered_set<int> ss;
    int n;
    int seen_same;
    
    n = 0;
    seen_same = 0;
    for (int i = 0; i < nums.size(); i++) {
        if (nums[i] > target)
            continue;
        if (nums[i] == (target - nums[i]) && seen_same < 2) {
            seen_same++;
            if (seen_same == 2)
                n++;
            continue;
        }
        if (ss.find(nums[i]) == ss.end()) { // haven't seen it
            if (ss.find(target - nums[i]) != ss.end()) // have seen the complement
                n++;
            ss.insert(nums[i]);
        }
    }
    
    //cout << "returning " << n << endl;
    
    return n;
}

static void test_prog1_specific_case1(void) {
    vector<int> v;
    
    v = {1, 1, 2, 45, 46, 46};
    assert(two_sum_unique_pairs(v, 47) == 2);
    
    v = {4, 4, 4};
    assert(two_sum_unique_pairs(v, 8) == 1);
    
    v = {4};
    assert(two_sum_unique_pairs(v, 8) == 0);
    
    v = {1, 5, 1, 5};
    assert(two_sum_unique_pairs(v, 6) == 1);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
