#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

// XXX: this is the right idea but still not passing on leetcode
// i assumed timestamps were sorted here, but they're not
// i'd also have to deal with double counting the same pattern for single
// users, too

static void dump_vector(vector<string> v) {
    for (int i = 0; i < v.size(); i++)
        cout << v[i] << " ";
    cout << endl;
}

typedef tuple<string, string, string> sss_t;

static unsigned int ht_default_hash(void *key, size_t keysz) {
    unsigned int hash;
    char *p;

    p = (char *)key;
    hash = 2166136261;
    
    for (int i = 0; i < keysz; i++)
        hash = (hash ^ p[i]) * 16777619;

    return hash;
}

// specialize std::hash for sss_t
namespace std {
template<>
class hash<sss_t> {
public:
    size_t operator()(const sss_t& k) const {
        string s(get<0>(k) + get<1>(k) + get<2>(k));
        return ht_default_hash((void *)s.data(), s.size());
    }
};
}

static void update_wsl(vector<string>& wsl, string& ws, unordered_map<sss_t, int>& pattern_counts, sss_t& out, int& max_count) {
    wsl.push_back(ws);
    if (wsl.size() > 2) {
        for (int i = 0; i < wsl.size() - 2; i++) {
            for (int j = i + 1; j < wsl.size() - 1; j++) {
                sss_t tup = make_tuple(wsl[i], wsl[j], ws);
                auto search = pattern_counts.find(tup);
                if (search != pattern_counts.end()) {
                    search->second++;
                    if ((search->second > max_count) || (search->second == max_count && search->first < out)) {
                        max_count = search->second;
                        out = search->first;
                    }
                } else {
                    pattern_counts.insert({tup, 1});
                    if (max_count == 0 || (max_count == 1 && tup < out)) {
                        max_count = 1;
                        out = tup;
                    }
                }
            }
        }
    }
}

vector<string> mostVisitedPattern(vector<string>& un, vector<int>& ts, vector<string>& ws) {
    unordered_map<string, vector<string>> uws;
    unordered_map<sss_t, int> pattern_counts;
    sss_t curr_max;
    vector<string> wsl, output;
    int max_count;
    
    max_count = 0;
    for (int i = 0; i < un.size(); i++) {
        auto search = uws.find(un[i]);
        if (search != uws.end()) {
            update_wsl(search->second, ws[i], pattern_counts, curr_max, max_count);
        } else {
            wsl.clear();
            wsl.push_back(ws[i]);
            uws.insert({un[i], wsl});
        }
    }
    
    for (auto it = pattern_counts.begin(); it != pattern_counts.end(); it++) {
        cout << get<0>(it->first) << " " << get<1>(it->first) << " " << get<2>(it->first) << " " << it->second << endl;
    }
    
    output.push_back(get<0>(curr_max));
    output.push_back(get<1>(curr_max));
    output.push_back(get<2>(curr_max));
    
    return output;
}

static void test_prog1_specific_case1(void) {
    vector<string> usernames, websites;
    vector<int> times;
    
    usernames = {"joe","joe","joe","james","james","james","james","mary","mary","mary"};
    times = {1,2,3,4,5,6,7,8,9,10};
    websites = {"home","about","career","home","cart","maps","home","home","about","career"};
    dump_vector(mostVisitedPattern(usernames, times, websites));
    
    usernames = {"ua","ua","ua","ub","ub","ub"};
    times = {1,2,3,4,5,6};
    websites = {"a","b","a","a","b","c"};
    dump_vector(mostVisitedPattern(usernames, times, websites));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
