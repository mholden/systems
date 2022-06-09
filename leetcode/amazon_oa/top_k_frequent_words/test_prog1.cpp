#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <set>

using namespace std;

static void dump_vector(vector<string> v) {
    cout << "v: ";
    for (int i = 0; i < v.size(); i++)
        cout << v[i] << " ";
    cout << endl;
}

struct scmp {
    bool operator() (pair<int, string> p1, pair<int, string> p2) const {
        if (p1.first > p2.first) {
            return true;
        } else if (p1.first < p2.first ){
            return false;
        } else { // ==
            if (p1.second > p2.second)
                return false;
            else
                return true;
        }
    }
};

static void dump_set(set<pair<int, string>, scmp>& s) {
    cout << "s: ";
    for (auto it = s.begin(); it != s.end(); it++)
        cout << "{" << (*it).first << ", " << (*it).second << "} ";
    cout << endl;
}

vector<string> topKFrequent(vector<string>& words, int k) {
    vector<string> topk;
    unordered_map<string, int> um;
    set<pair<int, string>, scmp> s;
    int i;
    
    for (int i = 0; i < words.size(); i++) {
        auto search = um.find(words[i]);
        if (search != um.end()) { // already an entry there
            search->second++;
        } else { // this is the first time we've seen it
            um.insert({words[i], 1});
        }
    }
    
    for (auto it = um.begin(); it != um.end(); it++)
        s.insert({it->second, it->first});
    
    //dump_set(s);
    
    i = k;
    for (auto it = s.begin(); (i > 0) && it != s.end(); it++, i--)
        topk.push_back((*it).second);
    
    return topk;
}

static void test_prog1_specific_case1(void) {
    vector<string> v;
    
    v = {"i","love","leetcode","i","love","coding"};
    dump_vector(topKFrequent(v, 4));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
