#include <vector>
#include <iostream>
#include <assert.h>

// https://cybergeeksquad.co/2021/06/shopping-options-amazon-online-assessment.html

using namespace std;

vector<vector<int>> so(int target);

static void dump_solution(vector<vector<int>> &s) {
    cout << "nsolutions: " << s.size() << endl;
    for (int i=0; i < s.size(); i++) {
        cout << "solution[ " << i << "]: ";
        for (int j=0; j < s[i].size(); j++) {
            cout << s[i][j] << " ";
        }
        cout << endl;
    }
}

static void test_so_specific_case1(void) {
    vector<vector<int>> s;
    s = so(5);
    dump_solution(s);
}

static void test_so(void) {
    test_so_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_so();
    return 0;
}
