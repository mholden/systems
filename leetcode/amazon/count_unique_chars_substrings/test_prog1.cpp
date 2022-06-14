#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <set>
#include <iostream>
#include <string.h>

using namespace std;

// O(n) time O(n) space
int _uniqueLetterString(string s) {
    set<int> chars[26];
    int unique, prev, nf, nb;
    
    for (int i = 0; i < s.size(); i++)
        chars[s[i] - 'A'].insert(i);
    
#if 0
    for (int i = 0; i < 26; i++) {
        if (chars[i].size()) {
            cout << "chars[" << i << "]: ";
            for (auto it = chars[i].begin(); it != chars[i].end(); it++)
                cout << *it << " ";
            cout << endl;
        }
    }
#endif
    
    unique = 0;
    for (int i = 0; i < 26; i++) {
        if (chars[i].size()) {
            prev = -1;
            for (auto it = chars[i].begin(); it != chars[i].end(); prev = *it, it++) {
                if (next(it) != chars[i].end())
                    nf = *next(it) - *it;
                else
                    nf = s.size() - *it;
                nb = *it - prev;
                //cout << "nf " << nf << " nb " << nb << endl;
                unique += nf * nb;
            }
        }
    }
    
    return unique;
}

// O(n) time O(1) space
int uniqueLetterString(string s) {
    int chars[26][2];
    int unique;
    
    memset(chars, -1, 26 * 2 * sizeof(int));
    unique = 0;
    for (int i = 0; i < s.size(); i++) {
        if (chars[s[i] - 'A'][1] >= 0)
            unique += (i - chars[s[i] - 'A'][1]) * (chars[s[i] - 'A'][1] - chars[s[i] - 'A'][0]);
        chars[s[i] - 'A'][0] = chars[s[i] - 'A'][1];
        chars[s[i] - 'A'][1] = i;
    }

#if 0
    cout << unique << endl;
    for (int i = 0; i < 26; i++)
        cout << chars[i][0] << " " << chars[i][1] << endl;
#endif
    
    for (int i = 0; i < 26; i++) {
        if (chars[i][0] >= 0)
            unique += (s.size() - chars[i][1]) * (chars[i][1] - chars[i][0]);
        else if (chars[i][1] >= 0)
            unique += (s.size() - chars[i][1]) * (chars[i][1] + 1);
    }
    
    return unique;
}

static void test_prog1_specific_case1(void) {
    assert(uniqueLetterString("ABC") == 10);
    assert(uniqueLetterString("ABA") == 8);
    assert(uniqueLetterString("ABACA") == 24);
    assert(uniqueLetterString("LEETCODE") == 92);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
