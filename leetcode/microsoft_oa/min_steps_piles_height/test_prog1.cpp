#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <iostream>

using namespace std;

static void dump_ms(multiset<int>& ms) {
    cout << "ms: ";
    for (auto it = ms.crbegin(); it != ms.crend(); ++it)
        cout << *it << " ";
    cout << endl;
}

//
// XXX: this isn't the best solution. notice that if you iteratie in order,
// max you'll add is n-1-i. if there are duplicates of that number
//
int min_steps(vector<int> nums) {
    multiset<int> ms;
    int highest, second_highest, steps;
    
    for (int i = 0; i < nums.size(); i++)
        ms.insert(nums[i]);
    
    steps = 0;
    while (1) {
        highest = 0;
        second_highest = 0;
        //cout << "loop start: " << endl;
        //dump_ms(ms);
        for (auto it = ms.crbegin(); it != ms.crend(); ++it) {
            if (!highest) {
                highest = *it;
                ms.erase(next(it).base());
            }
            if (!second_highest && *it != highest) {
                second_highest = *it;
                break;
            }
        }
        if (!second_highest) // we're done
            break;
        //cout << "highest " << highest << " second highest " << second_highest << endl;
        ms.insert(second_highest);
        steps++;
    }
    
    return steps;
}

static void test_prog1_specific_case1(void) {
    vector<int> v = {5, 2, 1};
    assert(min_steps(v) == 3);
    
    v = {5, 4, 3, 1};
    assert(min_steps(v) == 6);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
