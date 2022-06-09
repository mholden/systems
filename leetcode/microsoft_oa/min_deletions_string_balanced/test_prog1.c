#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void _min_deletions(char *s, int slen, int *min, int *nas) {
    int this_min, this_nas, sub_min, sub_nas;
    
    assert(slen > 0);
    if (slen == 1) {
        this_min = 0;
        if (s[0] == 'a') {
            this_nas = 1;
        } else {
            assert(s[0] == 'b');
            this_nas = 0;
        }
        goto out;
    }
    
    _min_deletions(s + 1, slen - 1, &sub_min, &sub_nas);
    if (s[0] == 'a') {
        this_min = sub_min;
        this_nas = sub_nas + 1;
    } else {
        assert(s[0] == 'b');
        if (sub_nas < sub_min + 1)
            this_min = sub_nas;
        else
            this_min = sub_min + 1;
        this_nas = sub_nas;
    }
    
out:
    *min = this_min;
    if (nas)
        *nas = this_nas;
    
    return;
}

int min_deletions(char *s) {
    int min, slen;
    slen = strlen(s);
    _min_deletions(s, slen, &min, NULL);
    return min;
}

static void test_prog1_specific_case1(void) {
    char *s;
    
    s = "aababbab";
    assert(min_deletions(s) == 2);
    
    s = "bbaaaaabb";
    assert(min_deletions(s) == 2);
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
