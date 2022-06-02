#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>
#include <unordered_set>

using namespace std;

class Solution {
public:
    int minDeletions(string s) {
        size_t freqs[26] = { 0 };
        unordered_set<int> counts;
        int ndeletions = 0;
        
        for (int i = 0; i < s.size(); i++)
            freqs[s[i] - 'a']++;
#if 0
        for (int i = 0; i < 26; i++) {
            if (freqs[i])
                printf("freq[%d]: %d\n", i, freqs[i]);
        }
#endif
        for (int i = 0; i < 26; i++) {
            while (freqs[i] && (counts.find(freqs[i]) != counts.end())) {
                ndeletions++;
                freqs[i]--;
            }
            if (freqs[i])
                counts.insert(freqs[i]);
        }
        
        return ndeletions;
    }
};

static void test_prog1_specific_case1(void) {
    Solution s;
    string str = "aaabbbcc";

    assert(s.minDeletions(str) == 2);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
