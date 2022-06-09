#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void dump_bt(int **bt, int btsz) {
    for (int i = 0; i < btsz; i++) {
        printf("[%d, %d]\n", bt[i][0], bt[i][1]);
    }
}

static int qs_cmp(const void *bt1, const void *bt2) {
    int *_bt1, *_bt2;
    
    _bt1 = *(int **)bt1;
    _bt2 = *(int **)bt2;
    
    //printf("qs_cmp: comparing [%d, %d] [%d, %d]\n", _bt1[0], _bt1[1], _bt2[0], _bt2[1]);
    
    if (_bt1[1] < _bt2[1])
        return 1;
    else if (_bt2[1] < _bt1[1])
        return -1;
    else // == 0
        return 0;
}

int maximum_units(int** bt, int btsz, int* btcsz, int ts) {
    int curr_nboxes = 0, curr_nunits = 0, curr_ind = 0, nboxes, *bti;
    
    qsort(bt, btsz, sizeof(int *), qs_cmp);
    //dump_bt(bt, btsz);
    
    for (int i = 0; i < btsz; i++) {
        if (curr_nboxes >= ts) // can't fit any more
            break;
        
        if (curr_nboxes + bt[i][0] <= ts) { // add all these boxes
            nboxes = bt[i][0];
        } else { // too many boxes, add as many as you can
            nboxes = ts - curr_nboxes;
        }
        
        curr_nunits += nboxes * bt[i][1];
        curr_nboxes += nboxes;
    }
    
    //printf("maximum_units: returning %d\n", curr_nunits);
    
    return curr_nunits;
}

int maximumUnits(int** boxTypes, int boxTypesSize, int* boxTypesColSize, int truckSize){
    //dump_bt(boxTypes, boxTypesSize);
    return maximum_units(boxTypes, boxTypesSize, boxTypesColSize, truckSize);
}

