#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int prog1(void);

static void test_prog1_specific_case1(void) {
    prog1();
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
