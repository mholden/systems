#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

// XXX: this works and is my best solution

static int **difficulties;

static int _min_difficulty(vector<int>& jd, int ind, int days_left) {
    int min = 0, this_difficulty, difficulty;
    
    //printf("_min_difficulties ind %d days_left %d\n", ind, days_left);
    
    if (difficulties[days_left - 1][ind] >= 0) {
        min = difficulties[days_left - 1][ind];
        goto out;
    }
    
    if (days_left == 1) {
        this_difficulty = 0;
        for (int i = ind; i < jd.size(); i++) {
            if (jd[i] > this_difficulty)
                this_difficulty = jd[i];
        }
        min = this_difficulty;
        difficulties[days_left - 1][ind] = min;
        goto out;
    }

    min = -1;
    this_difficulty = 0;
    for (int i = ind; i <= jd.size() - days_left; i++) {
        if (jd[i] > this_difficulty)
            this_difficulty = jd[i];
        difficulty = this_difficulty + _min_difficulty(jd, i + 1, days_left - 1);
        if (min < 0 || difficulty < min)
            min = difficulty;
    }
    
    difficulties[days_left - 1][ind] = min;
    
out:
    //printf("_min_difficulties ind %d days_left %d returning %d\n", ind, days_left, min);
    
    return min;
}

int minDifficulty(vector<int>& jd, int days) {
    if (jd.size() < days)
        return -1;
    assert(difficulties = (int **)malloc(sizeof(int *) * days));
    for (int i = 0; i < days; i++) {
        assert(difficulties[i] = (int *)malloc(sizeof(int) * jd.size()));
        memset(difficulties[i], -1, sizeof(int) * jd.size());
    }
    return _min_difficulty(jd, 0, days);
}

static void test_prog1_specific_case1(void) {
    vector<int> v;
    
    v = {6, 5, 4, 3, 2, 1};
    assert(minDifficulty(v, 2) == 7);
#if 1
    v = {10, 6, 4, 1, 12, 2, 14};
    printf("md %d\n", minDifficulty(v, 3));
    
    v = {380,302,102,681,863,676,243,671,651,612,162,561,394,856,601,30,6,257,921,405,716,126,158,476,889,699,668,930,139,164,641,801,480,756,797,915,275,709,161,358,461,938,914,557,121,964,315};
    printf("md %d\n", minDifficulty(v, 10));
#endif
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
