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
    queue<pair<TreeNode *, int>> fifo;
    pair<TreeNode *, int> fe;
    int ngood, max;;
    
    fifo.push({root, root->val});
    
    ngood = 1; // root's always good
    while (!fifo.empty()) {
        fe = fifo.front();
        fifo.pop();
        if (fe.first->left) {
            max = fe.second;
            if (fe.first->left->val >= fe.second) {
                ngood++;
                if (fe.first->left->val > max)
                    max = fe.first->left->val;
            }
            fifo.push({fe.first->left, max});
        }
        if (fe.first->right) {
            max = fe.second;
            if (fe.first->right->val >= fe.second) {
                ngood++;
                if (fe.first->right->val > max)
                    max = fe.first->right->val;
            }
            fifo.push({fe.first->right, max});
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
