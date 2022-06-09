#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

std::string longest_string(std::string s) {
    string output = "";
    int sind, eind, same, mxind, mx;
    
    sind = 0;
    mx = 0;
    while (1) {
        eind = sind + 1;
        same = 1;
        while (same < 3 && eind < s.size()) {
            if (s[eind] == s[sind])
                same++;
            else
                same = 0;
            eind++;
        }
        //cout << "same " << same << " sind " << sind << " eind " << eind << endl;
        if (same < 3) { // must have stopped because eind >= s.size()
            assert(eind >= s.size());
            eind -= 1;
            if (eind - sind + 1 > mx) {
                mx = eind - sind + 1;
                mxind = sind;
            }
            break;
        }
        eind -= 2;
        if (eind - sind + 1 > mx) {
            mx = eind - sind + 1;
            mxind = sind;
        }
        sind = eind;
        //cout << "mxind " << mxind << " mx " << mx << endl;
    }
    
    if (mx)
        output = s.substr(mxind, mx);
    
    //cout << "returning " << output << endl;
    
    return output;
}

static void test_prog1_specific_case1(void) {
    assert(longest_string("aabbaaaaabb") == "aabbaa");
    assert(longest_string("aabbaaaaa") == "aabbaa");
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
