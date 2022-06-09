#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <iostream>

using namespace std;

typedef pair<double, int> pqe_t;

double maxAverageRatio(vector<vector<int>>& classes, int extraStudents) {
    priority_queue<pqe_t> pq;
    pqe_t next;
    int ind;
    double _new, old, avg, pass_ratio, sum;
    
    for (int i = 0; i < classes.size(); i++) {
        if (classes[i][0] < classes[i][1]) {
            //cout << "adding {" << classes[i][1] << ", " << i << "} to pq" << endl;
            old = (double)classes[i][0] / (double)classes[i][1];
            _new = (double)(classes[i][0] + 1) / (double)(classes[i][1] + 1);
            pq.push({_new - old, i});
        }
    }
    
    if (pq.empty()) { // all classes are full
        avg = 1;
        goto out;
    }
    
    while (extraStudents) {
        next = pq.top();
        ind = next.second;
        pq.pop();
        classes[ind][0]++;
        classes[ind][1]++;
        old = (double)classes[ind][0] / (double)classes[ind][1];
        _new = (double)(classes[ind][0] + 1) / (double)(classes[ind][1] + 1);
        pq.push({_new - old, ind});
        extraStudents--;
    }
    
    sum = 0;
    for (int i = 0; i < classes.size(); i++) {
        sum += (double)classes[i][0] / (double)classes[i][1];
    }
    avg = sum / classes.size();
    
out:
    return avg;
}

static void test_prog1_specific_case1(void) {
    vector<vector<int>> cs;
    vector<int> c;
    int es;
    
    c.push_back(1);
    c.push_back(2);
    cs.push_back(c);
    
    c.clear();
    c.push_back(3);
    c.push_back(5);
    cs.push_back(c);
    
    c.clear();
    c.push_back(2);
    c.push_back(2);
    cs.push_back(c);
    
    printf("max average: %f\n", maxAverageRatio(cs, 2));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
