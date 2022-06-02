#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <unordered_set>
#include <iostream>

using namespace std;

int largest_k(std::vector<int> nums) {
    int max, pos;
    unordered_set<int> ps, ns;
    
    max = 0;
    for (int i = 0; i < nums.size(); i++) {
        if (nums[i] > 0) {
            if (ns.find(nums[i]) != ns.end() && nums[i] > max)
                max = nums[i];
            ps.insert(nums[i]);
        } else if (nums[i] < 0) {
            pos = -nums[i];
            if (ps.find(pos) != ps.end() && pos > max)
                max = pos;
            ns.insert(pos);
        }
    }
    
    return max;
}

static void test_prog1_specific_case1(void) {
    vector<int> v = {3, 2, -2, 5, -3};
    assert(largest_k(v) == 3);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
