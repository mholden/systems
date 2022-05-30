#include <vector>
#include <iostream>
#include <assert.h>

//
// XXX: wrote this to see if i could write it faster than using C
// result: nope. i don't know c++ well enough. i'm 90 mins in here
// and this doesn't even work
//

using namespace std;

vector<vector<vector<int>>> solutions; // array of all solutions

void dump_vv(vector<vector<int>>& vv) {
    for (int i = 0; i < vv.size(); i++) {
        cout << "solution[" << i << "] size " << vv[i].size() << " { ";
        for (int j = 0; j < vv[i].size(); j++) {
            cout << vv[i][j] << " ";
        }
        cout << "}" <<  endl;
    }
}

void dump_solutions() {
    cout << "** global solutions: " << endl;
    for (int i = 0; i < solutions.size(); i++) {
        cout << "solutions[" << i << "]" << endl;
        dump_vv(solutions[i]);
    }
}



void copy_sub_solution_with_index(vector<vector<int>>& s, vector<vector<int>>& ss, int index) {
    for (int i = 0; i < ss.size(); i++) {
        s.push_back(ss[i]);
        s[s.size()].push_back(index);
    }
}

bool v_contains_le(vector<int> v, int exclude) {
    for (int i = 0; i < v.size(); i++) {
        if (v[i] <= exclude)
            return true;
    }
    return false;
}

void copy_solution_with_exclusion(vector<vector<int>>& s, vector<vector<int>>& ss, int exclude) {
    //cout << "copy_solution_with_exclusion: " << endl;
    //dump_vv(ss);
    for (int i = 0; i < ss.size(); i++) {
        if (!v_contains_le(ss[i], exclude)) {
            s.push_back(ss[i]);
        }
    }
}

vector<vector<int>> recurse(int target, int exclude) {
    vector<vector<int>> s, sr;
    vector<int> t;
    
    cout << "recurse: " << target << " " << exclude << endl;
    dump_solutions();
    
    if (solutions.size() >= target) {
        copy_solution_with_exclusion(s, solutions[target-1], exclude);
        goto out;
    }
    
    for (int i = 1; i < target; i++) {
        sr = recurse(target-i, i-1);
        if (sr.size())
            copy_sub_solution_with_index(s, sr, i);
    }
    
    // add target vector
    t.push_back(target);
    s.push_back(t);
    
    // add solution to solutions
    solutions.push_back(s);
    
out:
    return s;
}

vector<vector<int>> so(int target) {
    vector<vector<int>> output;
    output = recurse(target, 0);
    dump_vv(output);
    return output;
}
