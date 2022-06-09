#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

struct ListNode {
      int val;
      ListNode *next;
      ListNode() : val(0), next(nullptr) {}
      ListNode(int x) : val(x), next(nullptr) {}
      ListNode(int x, ListNode *next) : val(x), next(next) {}
};

static void dump_list(ListNode *l) {
    cout << "list: ";
    while (l) {
        cout << l->val << " ";
        l = l->next;
    }
    cout << endl;
}

// solution that returns a new list
ListNode* _mergeTwoLists(ListNode* l1, ListNode* l2) {
    ListNode *ol = NULL, *c1, *c2, *co, *nn;
    
    //dump_list(l1);
    //dump_list(l2);

    c1 = l1;
    c2 = l2;
    co = ol;
    while (c1 || c2) {
        if (!c2 || (c1 && c1->val < c2->val)) {
            nn = new ListNode(c1->val);
            c1 = c1->next;
        } else {
            nn = new ListNode(c2->val);
            c2 = c2->next;
        }
        if (!co) // root node
            ol = nn;
        else
            co->next = nn;
        co = nn;
    }
    
    return ol;
}

// in place solution
ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
    ListNode *ol = NULL, *c1, *c2, *co;
    
    //dump_list(l1);
    //dump_list(l2);

    c1 = l1;
    c2 = l2;
    while (c1 || c2) {
        if (!c2 || (c1 && c1->val < c2->val)) {
            if (!ol) {
                ol = c1;
                co = c1;
            } else {
                co->next = c1;
                co = c1;
            }
            c1 = c1->next;
        } else {
            if (!ol) {
                ol = c2;
                co = c2;
            } else {
                co->next = c2;
                co = c2;
            }
            c2 = c2->next;
        }
    }
    
    return ol;
}


static void test_prog1_specific_case1(void) {
    ListNode l1[3], l2[3];
    
    l1[2].val = 4;
    l1[1].val = 2;
    l1[1].next = &l1[2];
    l1[0].val = 1;
    l1[0].next = &l1[1];
    
    l2[2].val = 4;
    l2[1].val = 3;
    l2[1].next = &l2[2];
    l2[0].val = 1;
    l2[0].next = &l2[1];
    dump_list(mergeTwoLists(&l1[0], &l2[0]));
    //dump_list(mergeTwoLists(&l2[0], &l1[0]));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
