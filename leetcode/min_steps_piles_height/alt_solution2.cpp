#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>

using namespace std;

static void dump_nums(int *nums, size_t nsz) {
    printf("nums: ");
    for (int i = 0; i < nsz; i++)
        printf("%d ", nums[i]);
    printf("\n");
}

static int qs_cmp(const void *n1, const void *n2) {
    int *_n1, *_n2;
    
    _n1 = (int *)n1;
    _n2 = (int *)n2;
    
    if (*_n1 > *_n2)
        return 1;
    else if (*_n2 > *_n1)
        return -1;
    else // ==
        return 0;
}

int min_steps(vector<int> nums) {
    int steps;
    
    //dump_nums(nums.data(), nums.size());
    qsort(nums.data(), nums.size(), sizeof(int), qs_cmp);
    //dump_nums(nums.data(), nums.size());
    
    steps = 0;
    for (int i = 0; i < nums.size() - 1; i++) {
        if (nums[i + 1] != nums[i])
            steps += nums.size() - 1 - i;
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
