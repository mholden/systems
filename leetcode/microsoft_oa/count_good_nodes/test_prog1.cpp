#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <queue>

using namespace std;

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

int goodNodes(TreeNode* root) {
    vector<pair<int, int>> visited;
    queue<pair<TreeNode *, pair<int, int>>> fifo;
    pair<TreeNode *, pair<int, int>> fe;
    int ngood, visited_index, back_pointer;
    
    back_pointer = visited.size() - 1;
    visited_index = visited.size();
    fifo.push({root, {back_pointer, visited_index}});
    visited.push_back({root->val, back_pointer});
    
    ngood = 0;
    while (!fifo.empty()) {
        fe = fifo.front();
        fifo.pop();
        
        // check if node is good
        visited_index = fe.second.first;
        while (visited_index >= 0) {
            if (visited[visited_index].first > fe.first->val)
                break;
            visited_index = visited[visited_index].second;
        }
        if (visited_index < 0)
            ngood++;
        
        // and add our children to the fifo
        back_pointer = fe.second.second;
        if (fe.first->left) {
            visited_index = visited.size();
            fifo.push({fe.first->left, {back_pointer, visited_index}});
            visited.push_back({fe.first->left->val, back_pointer});
        }
        if (fe.first->right) {
            visited_index = visited.size();
            fifo.push({fe.first->right, {back_pointer, visited_index}});
            visited.push_back({fe.first->right->val, back_pointer});
        }
    }
    
    return ngood;
}

static void test_prog1_specific_case1(void) {
    TreeNode n[6];
    
    n[5].val = 5;
    n[4].val = 1;
    n[3].val = 3;
    n[2].val = 4;
    n[2].right = &n[5];
    n[2].left = &n[4];
    n[1].val = 1;
    n[1].left = &n[3];
    n[0].val = 3;
    n[0].right = &n[2];
    n[0].left = &n[1];
    assert(goodNodes(&n[0]) == 4);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
