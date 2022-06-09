#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <set>

using namespace std;

int min_steps(vector<int> nums) {
    multiset<int> ms;
    int steps;
    
    for (int i = 0; i < nums.size(); i++)
        ms.insert(nums[i]);
    
    steps = 0;
    auto it = ms.begin();
    for (int i = 0; i < nums.size() - 1; i++) {
        if (*next(it) != *it)
            steps += nums.size() - 1 - i;
        it++;
    }
    
    return steps;
}

static void test_prog1_specific_case1(void) {
    vector<int> v = {5, 2, 1};
    assert(min_steps(v) == 3);
    
    v = {5, 4, 3, 1};
    assert(min_steps(v) == 6);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
