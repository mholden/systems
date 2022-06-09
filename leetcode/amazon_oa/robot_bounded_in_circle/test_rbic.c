#include <stdio.h>
#include <assert.h>

int robot_bounded_in_circle(char * instructions);

static void test_rbic_specific_cases(void) {
    char *s;
    
    s = "GGLLGG";
    assert(robot_bounded_in_circle(s));
    
    s = "GG";
    assert(!robot_bounded_in_circle(s));
    
    s = "GL";
    assert(robot_bounded_in_circle(s));
}

static void test_rbic(void) {
    test_rbic_specific_cases();
}

int main(int argc, const char *argv[]) {
    test_rbic();
    return 0;
}
