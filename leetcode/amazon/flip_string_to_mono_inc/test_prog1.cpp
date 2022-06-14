#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int minFlipsMonoIncr(string s) {
    int _min, nzeros, nones;
    
    _min = 0;
    nzeros = 0;
    nones = 0;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == '0') {
            _min = min(_min + 1, nones);
            _min = min(_min, nzeros + 1);
            nzeros++;
        } else {
            _min = min(_min, nones + 1);
            _min = min(_min, nzeros);
            nones++;
        }
    }
        
    return _min;
}

static void test_prog1_specific_case1(void) {
    assert(minFlipsMonoIncr("00011000") == 2);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
