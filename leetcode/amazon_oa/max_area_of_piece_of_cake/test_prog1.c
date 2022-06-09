#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <inttypes.h>

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

int maxArea(int h, int w, int* horizontalCuts, int horizontalCutsSize, int* verticalCuts, int verticalCutsSize) {
    int maxv, maxh;
    uint64_t max;
    
    qsort(horizontalCuts, horizontalCutsSize, sizeof(int), qs_cmp);
    qsort(verticalCuts, verticalCutsSize, sizeof(int), qs_cmp);
    
    maxv = verticalCuts[0];
    for (int i = 1; i < verticalCutsSize; i++) {
        if (verticalCuts[i] - verticalCuts[i-1] > maxv)
            maxv = verticalCuts[i] - verticalCuts[i-1];
    }
    if (w - verticalCuts[verticalCutsSize-1] > maxv)
        maxv = w - verticalCuts[verticalCutsSize-1];
    
    maxh = horizontalCuts[0];
    for (int i = 1; i < horizontalCutsSize; i++) {
        if (horizontalCuts[i] - horizontalCuts[i-1] > maxh)
            maxh = horizontalCuts[i] - horizontalCuts[i-1];
    }
    if (h - horizontalCuts[horizontalCutsSize-1] > maxh)
        maxh = h - horizontalCuts[horizontalCutsSize-1];
    
    //printf("maxh %" PRIu64 " maxv %" PRIu64 "\n", maxh, maxv);
    max = (uint64_t)maxh * (uint64_t)maxv % ((uint64_t)1e9 + 7);
    //printf("returning %" PRIu64 "\n", max);
    
    return max;
}

static void test_prog1_specific_case2(void) {
    int hc[1] = {2}, vc[1] = {2};
    assert(maxArea(1000000000, 1000000000, hc, 1, vc, 1) == 81);
}

static void test_prog1_specific_case1(void) {
    int hc[3] = {2, 1, 4}, vc[2] = {1,3};
    assert(maxArea(5, 4, hc, 3, vc, 2) == 4);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
    test_prog1_specific_case2();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
