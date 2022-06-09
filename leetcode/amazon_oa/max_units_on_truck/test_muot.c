#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int maximumUnits(int** boxTypes, int boxTypesSize, int* boxTypesColSize, int truckSize);

static void test_prog1_specific_case1(void) {
    int **bt;
    
    assert(bt = malloc(4 * sizeof(int *)));
    for (int i = 0; i < 4; i++)
        assert(bt[i] = malloc(2 * sizeof(int)));
    
    bt[0][0] = 5;
    bt[0][1] = 10;
    bt[1][0] = 2;
    bt[1][1] = 5;
    bt[2][0] = 4;
    bt[2][1] = 7;
    bt[3][0] = 3;
    bt[3][1] = 9;
    
    assert(maximumUnits(bt, 4, NULL, 10) == 91);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
