#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>

using namespace std;

static int sum(vector<int> v) {
    int sum = 0;
    for (int i = 0; i < v.size(); i++)
        sum += v[i];
    return sum;
}

vector<int> sumZero(int n) {
    vector<int> v;
    
    for (int i = 1; i <= n/2; i++) {
        v.push_back(i);
        v.push_back(-i);
    }

    if (n % 2)
        v.push_back(0);

    return v;
}

static void test_prog1_specific_case1(void) {
    for (int i = 0; i < 100; i++)
        assert(sum(sumZero(i)) == 0);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
