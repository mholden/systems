#include <iostream>
#include <vector>

using namespace std;

void dump_result(vector<int> result) {
    cout << "result: ";
    for (int i = 0; i < result.size(); i++) {
        cout << result[i] << " ";
    }
    cout << endl;
}

void print(vector<vector<int>> output) {
    cout << "output:" << endl;
    for (int i = 0; i < output.size(); i++) {
        cout << "[ ";
        for (int j = 0; j < output[i].size(); j++) {
            cout << output[i][j] << ", ";
        }
        cout << "]" << endl;
    }
}

void print_all_sum_rec(int target, int current_sum, int start, vector<vector<int>>& output, vector<int>& result) {
    cout << "print_all_sum_rec: target " << target << " current_sum " << current_sum << " start " << start <<  endl;
    print(output);
    dump_result(result);
    if (target == current_sum) {
        cout << "adding output" << endl;
        output.push_back(result);
    }
    for (int i = start; i < target; i++) {
        int temp_sum = current_sum + i;
        if (temp_sum <= target) {
            result.push_back(i);
            print_all_sum_rec(target, temp_sum, i, output, result);
            result.pop_back();
        } else {
            cout << "returning" << endl;
            return;
        }
    }
    cout << "returning normal" << endl;
}

vector<vector<int>> print_all_sum(int target) {
    vector<vector<int>> output;
    vector<int> result;
    print_all_sum_rec(target, 0, 1, output, result);
    return output;
}

int main (int argc, const char *argv[]) {
    int n = 5;
    vector<vector<int>> result = print_all_sum(n);
    print(result);
    return 0;
}
