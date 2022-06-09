#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

using namespace std;

static void dump_partitions(vector<vector<int>> partitions) {
    vector<int> v;
    for (int i = 0; i < partitions.size(); i++) {
        v = partitions[i];
        cout << "partition " << i << ": ";
        for (int j = 0; j < v.size(); j++)
            cout << v[j] << " ";
        cout << endl;
    }
}

static int vector_sum(vector<int>& v) {
    int sum;
    
    sum = 0;
    for (int i = 0; i < v.size(); i++)
        sum += v[i];
    
    return sum;
}

vector<vector<int>> partition(vector<int>& input, int n) {
    vector<vector<int>> partitions;
    vector<int> partition;
    int sum, target, bind, find, psum;
    
    sum = vector_sum(input);
    target = sum / n + sum % n;
    
    find = 0;
    bind = input.size() - 1;
    for (int i = 0; i < n; i++) {
        psum = 0;
        partition.clear();
        while (bind >= 0) {
            if (psum + input[bind] > target)
                break;
            partition.push_back(input[bind]);
            psum += input[bind];
            bind--;
        }
        while (find < bind) {
            if (psum + input[find] > target)
                break;
            partition.push_back(input[find]);
            psum += input[find];
            find++;
        }
        if (partition.empty() && bind >= 0) { // put at least 1 in each partition
            partition.push_back(input[bind--]);
        }
        partitions.push_back(partition);
    }
    
    return partitions;
}

static void test_prog1_specific_case1(void) {
    vector<int> v;
    
    v = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    dump_partitions(partition(v, 3));
    
    v = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    dump_partitions(partition(v, 2));
    
    v = {1, 10, 10};
    dump_partitions(partition(v, 3));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
