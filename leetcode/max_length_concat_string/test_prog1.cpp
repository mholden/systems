#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

static void dump_solns(vector<string *>& solns) {
    for (int i = 0; i < solns.size(); i++) {
        cout << *solns[i] << endl;
    }
}

static void solns_add(vector<string *>& solns, string *s, int& max) {
    solns.push_back(s);
    if (s->length() > max)
        max = s->length();
}

static bool chars_unique(string& s1, string& s2) {
    int chars[26] = { 0 };
    for (int i = 0; i < s1.length(); i++) {
        chars[s1[i] - 'a'] = 1;
    }
    for (int i = 0; i < s2.length(); i++) {
        if (chars[s2[i] - 'a'])
            return false;
    }
    return true;
}

static bool chars_unique_single_string(string& s1) {
    int chars[26] = { 0 };
    for (int i = 0; i < s1.length(); i++) {
        if (chars[s1[i] - 'a'])
            return false;
        chars[s1[i] - 'a'] = 1;
    }
    return true;
}

static void _max_length(vector<string>& sa, size_t start, vector<string *>& solns, int& max) {
    string *s;
    //cout << "_max_length " << sa[start] << " start " << start << " max " << max << endl;
    //cout << "solns at start:" << endl;
    //dump_solns(solns);
    if (sa.size() - start > 1)
        _max_length(sa, start + 1, solns, max);
    
    if (!chars_unique_single_string(sa[start]))
        return;

    for (int i = 0; i < solns.size(); i++) {
        //cout << "strings " << sa[start] << " " << *solns[i] << endl;
        if (chars_unique(sa[start], *solns[i])) {
            s = new string(sa[start] + *solns[i]);
            solns_add(solns, s, max);
        }
    }
    
    s = new string(sa[start]);
    solns_add(solns, s, max);
    //cout << "solns at out:" << endl;
    //dump_solns(solns);
}

int maxLength(vector<string>& arr) {
    vector<string *> solns;
    int max = 0;
    
    _max_length(arr, 0, solns, max);
    
    // clean up
    for (int i = 0; i < solns.size(); i++)
        delete solns[i];
        
    return max;
}

static void test_prog1_specific_case1(void) {
    vector<string> v;
    
    v = {"cha", "r", "act", "ers"};
    assert(maxLength(v) == 6);
    
    v = {"aa", "bb"};
    assert(maxLength(v) == 0);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
