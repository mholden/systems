#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

char slowestKey(int* releaseTimes, int releaseTimesSize, char * keysPressed){
    int diff, max;
    char maxc;

    max = releaseTimes[0];
    maxc = keysPressed[0];
    for (int i = 1; i < releaseTimesSize; i++) {
        diff = releaseTimes[i] - releaseTimes[i-1];
        if (diff > max) {
            max = diff;
            maxc = keysPressed[i];
        } else if (diff == max && keysPressed[i] > maxc) {
            maxc = keysPressed[i];
        }
    }
        
    return maxc;
}

static void test_prog1_specific_case1(void) {
    int rt[4];
    char *kp = "cbcd";
    
    rt[0] = 9;
    rt[1] = 29;
    rt[2] = 49;
    rt[3] = 50;
    assert(slowestKey(rt, 4, kp) == 'c');
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
