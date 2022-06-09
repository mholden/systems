#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <unordered_set>
#include <queue>
#include <iostream>

using namespace std;

static void dump_visited(unordered_set<int>& visited) {
    cout << "visited:";
    for (auto it = visited.begin(); it != visited.end(); it++)
        cout << " " << *it;
    cout << endl;
}

bool jump_game(std::vector<int> arr, int start) {
    queue<int> fifo;
    unordered_set<int> visited;
    int fe, left, right;
    
    fifo.push(start);
    visited.insert(start);
    
    while (!fifo.empty()) {
        fe = fifo.front();
        //cout << "fe " << fe << " arr[fe] " << arr[fe] << endl;
        //dump_visited(visited);
        fifo.pop();
        if (arr[fe] == 0) // we got to 0, so we're done
            return true;
        left = fe - arr[fe];
        if (left >= 0 && visited.find(left) == visited.end()) {
            fifo.push(left);
            visited.insert(left);
        }
        right = fe + arr[fe];
        if (right < arr.size() && visited.find(right) == visited.end()) {
            fifo.push(right);
            visited.insert(right);
        }
    }
    
    return false;
}

static void test_prog1_specific_case1(void) {
    vector<int> v;
    
    v = { 3, 4, 2, 3, 0, 3, 1, 2, 1 };
    assert(jump_game(v, 7));
    
    v = { 3, 2, 1, 3, 0, 3, 1, 2, 1 };
    assert(!jump_game(v, 2));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
