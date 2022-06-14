#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <set>

using namespace std;

static void dump_vv(vector<vector<string>> vv) {
    for (int i = 0; i < vv.size(); i++) {
        for (int j = 0; j < vv[i].size(); j++) {
            cout << vv[i][j] << " ";
        }
        cout << endl;
    }
}

vector<vector<string>> suggestedProducts(vector<string>& products, string searchWord) {
    unordered_map<string, set<string>> pfht; // prefix hash table
    set<string> pt; // product tree
    vector<string> out_entry;
    vector<vector<string>> out;
    
    for (int i = 0; i < products.size(); i++) {
        for (int j = 0; j < products[i].size(); j++) {
            pfht[products[i].substr(0, j+1)].insert(products[i]);
        }
    }
    
#if 0
    for (auto hit = pfht.begin(); hit != pfht.end(); hit++) {
        cout << hit->first << ": ";
        for (auto trit = hit->second.begin(); trit != hit->second.end(); trit++)
            cout << *trit << " ";
        cout << endl;
    }
#endif
    
    for (int i = 0; i < searchWord.size(); i++) {
        out_entry.clear();
        pt = pfht[searchWord.substr(0, i+1)];
        for (auto trit = pt.begin(); trit != pt.end(); trit++) {
            out_entry.push_back(*trit);
            if (out_entry.size() == 3)
                break;
        }
        out.push_back(out_entry);
    }
    
    return out;
}

static void test_prog1_specific_case1(void) {
    vector<string> products;
    
    products = {"mobile","mouse","moneypot","monitor","mousepad"};
    dump_vv(suggestedProducts(products, "mouse"));
    
    products = {"havana"};
    dump_vv(suggestedProducts(products, "havana"));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
