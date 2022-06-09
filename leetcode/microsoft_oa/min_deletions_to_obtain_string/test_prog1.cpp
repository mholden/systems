#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

// XXX: this is wrong. see min_deletions_string_balanced
// this is a greedy algorithm, and it is not the optimal
// solution in some cases
int min_deletions(string inp) {
    int find, bind, min, ys, xs;
    
    find = 0;
    bind = inp.size() - 1;
    min = 0;
    while (find <= bind) {
        //cout << "start find " << find << " bind " << bind << endl;
        while (inp[find] == 'X' && find < inp.size())
            find++;
        while (inp[bind] == 'Y' && bind > 0)
            bind--;
        if (find >= bind) // done
            break;
        // if we're here, it means we have characters to delete
        ys = 0;
        xs = 0;
        while (inp[find + ys] == 'Y')
            ys++;
        while (inp[bind - xs] == 'X')
            xs++;
        //cout << "find " << find << " bind " << bind << " ys " << ys << " xs " << xs << endl;
        if (ys <= xs) {
            min += ys;
            find += ys;
        } else {
            min += xs;
            bind -= xs;
        }
        
    }
    
    //cout << "min: " << min << endl;
    
    return min;
}

static void test_prog1_specific_case1(void) {
    //assert(min_deletions("YXXXYXY") == 2);
    assert(min_deletions("YYXYXX") == 3);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
