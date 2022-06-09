#include <stdio.h>
#include <assert.h>

// https://cybergeeksquad.co/2021/06/shopping-options-amazon-online-assessment.html

int **so(int target, int *nsolutions, size_t **solution_lengths);

static void dump_solution(int **solution, int nsolutions, size_t *solution_lengths) {
    printf("nsolutions: %d\n", nsolutions);
    for (int i=0; i < nsolutions; i++) {
        printf("solution[%d]: ", i);
        for (int j=0; j < solution_lengths[i]; j++) {
            printf("%d ", solution[i][j]);
        }
        printf("\n");
    }
}

static void test_so_specific_case1(void) {
    int **result, nsolutions;
    size_t *solution_lengths;
    
    result = so(5, &nsolutions, &solution_lengths);
    dump_solution(result, nsolutions, solution_lengths);
}

static void test_so(void) {
    test_so_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_so();
    return 0;
}
