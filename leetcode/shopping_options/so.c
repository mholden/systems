#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

typedef struct solution {
    size_t nsolutions;
    int **solution;
    size_t *solution_lengths;
} solution_t;

// copy all solutions from 'from' to 'to'
int solution_add_solutions(solution_t *to, solution_t *from, int index) {
    int err;
    
    for (int i = 0; i < from->nsolutions; i++) {
        to->solution_lengths = realloc(to->solution_lengths, sizeof(size_t) * (to->nsolutions + 1));
        assert(to->solution_lengths); // TODO: deal with this
        to->solution_lengths[to->nsolutions] = from->solution_lengths[i] + 1;
        
        to->solution = realloc(to->solution, sizeof(int *) * (to->nsolutions + 1));
        assert(to->solution); // TODO: deal with this
        
        to->solution[to->nsolutions] = malloc(sizeof(int) * (from->solution_lengths[i] + 1));
        assert(to->solution[to->nsolutions]); // TODO: deal with this
        
        memcpy(to->solution[to->nsolutions], from->solution[i], from->solution_lengths[i] * sizeof(int));
        to->solution[to->nsolutions][from->solution_lengths[i]] = index;
        
        to->nsolutions++;
    }
    
    return 0;
    
error_out:
    return err;
}

int solution_contains_le(solution_t *s, int index, int exclude) {
    for (int i = 0; i < s->solution_lengths[index]; i++) {
        if (s->solution[index][i] <= exclude)
            return 1;
    }
    return 0;
}

int solution_copy(solution_t *from, solution_t *to, int exclude) {
    int err;
    
    for (int i = 0; i < from->nsolutions; i++) {
        if (!solution_contains_le(from, i, exclude)) {
            to->solution_lengths = realloc(to->solution_lengths, sizeof(size_t) * (to->nsolutions + 1));
            assert(to->solution_lengths); // TODO: deal with this
            to->solution_lengths[to->nsolutions] = from->solution_lengths[i];
            
            to->solution = realloc(to->solution, sizeof(int *) * (to->nsolutions + 1));
            assert(to->solution); // TODO: deal with this
            
            to->solution[to->nsolutions] = malloc(sizeof(int) * from->solution_lengths[i]);
            assert(to->solution[to->nsolutions]); // TODO: deal with this
            
            memcpy(to->solution[to->nsolutions], from->solution[i], from->solution_lengths[i] * sizeof(int));
            
            to->nsolutions++;
        }
    }
    
    return 0;
    
error_out:
    return err;
}

static void dump_solution(solution_t *s) {
    printf("nsolutions %d\n", s->nsolutions);
    for (int i=0; i < s->nsolutions; i++) {
        printf("solution[%d]: length %d solution: ", i, s->solution_lengths[i]);
        for (int j=0; j < s->solution_lengths[i]; j++) {
            printf("%d ", s->solution[i][j]);
        }
        printf("\n");
    }
}

static int gtarget;

static void dump_solutions(solution_t *s) {
    printf("** global solutions:\n");
    for (int i = 0; i < gtarget; i++) {
        dump_solution(&s[i]);
    }
}

solution_t *solutions;

solution_t *recurse(int target, int exclude) {
    solution_t *s = NULL, *sr, last;
    int err;
    
    s = malloc(sizeof(solution_t));
    if (!s)
        goto error_out;
    
    memset(s, 0, sizeof(solution_t));
    
    assert(target > 0);
    if (solutions[target-1].nsolutions) {
        err = solution_copy(&solutions[target-1], s, exclude);
        if (err)
            goto error_out;
        goto out;
    }
    
    assert(!exclude);
    for (int i = 1; i < target; i++) {
        sr = recurse(target-i, i-1);
        solution_add_solutions(s, sr, i);
        // solution_destroy(sr) XXX: memory leak until you do this. need to free sr memory
    }
    
    // add solution of length 1 that includes target and nothing else:
    
    s->solution_lengths = realloc(s->solution_lengths, sizeof(size_t) * (s->nsolutions + 1));
    assert(s->solution_lengths); // TODO: deal with this
    s->solution_lengths[s->nsolutions] = 1;
    
    s->solution = realloc(s->solution, sizeof(int *) * (s->nsolutions + 1));
    assert(s->solution); // TODO: deal with this
    
    s->solution[s->nsolutions] = malloc(sizeof(int));
    assert(s->solution[s->nsolutions]); // TODO: deal with this
    
    s->solution[s->nsolutions][0] = target;
    
    s->nsolutions++;
    
    // copy to global. this should do a deep copy instead of this 
    memcpy(&solutions[target-1], s, sizeof(solution_t));
    
out:
    return s;
    
error_out:
    if (s)
        free(s);
    
    return NULL;
}

// XXX the memory management here is a mess; i'm not freeing things
int **so(int target, int *nsolutions, size_t **solution_lengths) {
    solution_t *s;
    
    gtarget = target;
    
    solutions = malloc(sizeof(solution_t) * target);
    if (!solutions) {
        goto error_out;
    }
    memset(solutions, 0, sizeof(solution_t) * target);
    
    s = recurse(target, 0);
    if (!s)
        goto error_out;
    
    *nsolutions = s->nsolutions;
    *solution_lengths = s->solution_lengths;
    
    //dump_solutions(solutions);
    
    return s->solution;
    
error_out:
    return NULL;
}
