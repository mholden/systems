#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <string.h>
#include <algorithm>

using namespace std;

static void dump_vector(vector<string> v) {
    for (int i = 0; i < v.size(); i++)
        cout << v[i] << endl;
}

static bool is_letter_log(string &s) {
    char *_s = (char *)s.data();
    while (*_s++ != ' ');
    if (*_s >= 'a' && *_s <= 'z')
        return true;
    return false;
}

static bool ll_compare(string& s1, string &s2) {
    char *data1, *data2, *id1, *id2;
    int cmp;
    
    id1 = (char *)s1.data();
    id2 = (char *)s2.data();
    
    data1 = id1;
    while (*data1++ != ' ');
    
    data2 = id2;
    while (*data2++ != ' ');
    
    cmp = strcmp(data1, data2);
    if (cmp < 0)
        return true;
    else if (cmp > 0)
        return false;
    else { // ==
        // compare the identifiers
        cmp = strcmp(id1, id2);
        if (cmp < 0)
            return true;
        else
            return false;
    }
}

struct scmp {
    bool operator() (string s1, string s2) const {
        if (is_letter_log(s1)) {
            if (is_letter_log(s2))
                return ll_compare(s1, s2);
            else
                return true;
        } else {
            return false;
        }
    }
};

vector<string> reorderLogFiles(vector<string>& logs) {
    vector<string> out;
    
    out = logs;
    stable_sort(out.begin(), out.end(), scmp{});
    
    return out;
}

static void test_prog1_specific_case1(void) {
    vector<string> v;
    
    v = {"dig1 8 1 5 1","let1 art can","dig2 3 6","let2 own kit dig","let3 art zero","let6 art can"};
    dump_vector(v);
    cout << endl;
    dump_vector(reorderLogFiles(v));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
