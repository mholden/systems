#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

static void dump_(void) {
    
}

int _numPairsDivisibleBy60(int* time, int timeSize){
    int sum = 0;
    for (int i = 0; i < timeSize; i++) {
        for (int j = i + 1; j < timeSize; j++) {
            if (((time[i] + time[j]) % 60) == 0)
                sum++;
        }
    }
    //printf("numPairsDivisibleBy60: returning %d\n", sum);
    return sum;
}

// better solution:
int numPairsDivisibleBy60(int* time, int timeSize){
    int sum = 0, remainder, remainders[60] = { 0 };
    for (int i = 0; i < timeSize; i++) {
        remainder = time[i] % 60;
        sum += remainders[(60 - remainder) % 60];
        remainders[remainder]++;
    }
    //printf("numPairsDivisibleBy60: returning %d\n", sum);
    return sum;
}
