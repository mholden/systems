#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

using namespace std;

static int do_swaps(string& s, size_t ind1, size_t ind2, bool forward) {
    size_t cind;
    char tmp;
    int nswaps = 0;
    
    //cout << "do_swaps start; string: " << s << " ind1 " << ind1 << " ind2 " << ind2 << endl;
    
    assert(ind1 < ind2);
    if (forward) {
        cind = ind1;
        while (cind < ind2) {
            tmp = s[cind + 1];
            s[cind + 1] = s[cind];
            s[cind] = tmp;
            cind++;
            nswaps++;
        }
    } else {
        cind = ind2;
        while (cind > ind1) {
            tmp = s[cind - 1];
            s[cind - 1] = s[cind];
            s[cind] = tmp;
            cind--;
            nswaps++;
        }
    }
    
    //cout << "do_swaps end; string: " << s << endl;
    
    return nswaps;
}

typedef struct swap_data {
    size_t distance;
    size_t ind1;
    size_t ind2;
    bool forward;
} swap_data_t;

int min_swaps(string inp) {
    int nswaps, tswaps = 0;
    size_t find, bind, fsind, bsind, dst;
    swap_data_t sd;
    
    find = 0;
    bind = inp.size() - 1;
    while (find < bind) {
        //cout << "loop start; string: " << inp << " find " << find << " bind " << bind << endl;
        if (inp[find] != inp[bind]) {
            fsind = find + 1;
            bsind = bind - 1;
            nswaps = 0;
            memset(&sd, 0, sizeof(swap_data_t));
            while (fsind <= bsind) {
                if (inp[fsind] == inp[find]) {
                    dst = bind - fsind;
                    if (dst < sd.distance || !sd.distance) {
                        sd.distance = dst;
                        sd.ind1 = fsind;
                        sd.ind2 = bind;
                        sd.forward = true;
                    }
                }
                if (inp[fsind] == inp[bind]) {
                    dst = fsind - find;
                    if (dst < sd.distance || !sd.distance) {
                        sd.distance = dst;
                        sd.ind1 = find;
                        sd.ind2 = fsind;
                        sd.forward = false;
                    }
                }
                if (bsind > fsind) {
                    if (inp[bsind] == inp[find]) {
                        dst = bind - bsind;
                        if (dst < sd.distance || !sd.distance) {
                            sd.distance = dst;
                            sd.ind1 = bsind;
                            sd.ind2 = bind;
                            sd.forward = true;
                        }
                    }
                    if (inp[bsind] == inp[bind]) {
                        dst = bsind - find;
                        if (dst < sd.distance || !sd.distance) {
                            sd.distance = dst;
                            sd.ind1 = find;
                            sd.ind2 = bsind;
                            sd.forward = false;
                        }
                    }
                }
                fsind++;
                bsind--;
            }
            if (!sd.distance) // nothing to swap. no solution
                goto no_solution;
            nswaps = do_swaps(inp, sd.ind1, sd.ind2, sd.forward);
            tswaps += nswaps;
        }
        assert(inp[find] == inp[bind]);
        find++;
        bind--;
    }
    
    //cout << "min_swaps end: " << inp << " swaps: " << tswaps << endl;
    
    return tswaps;
    
no_solution:
    return -1;
}

static void test_prog1_specific_case1(void) {
    string s = "earccra";
    assert(min_swaps(s) == 3);
    
    s = "aabb";
    assert(min_swaps(s) == 2);
    
    s = "aacddcbab";
    assert(min_swaps(s) == 8);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
