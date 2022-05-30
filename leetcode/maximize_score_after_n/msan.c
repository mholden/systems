#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void dump_array(int *nums, int nsize) {
    for (int i = 0; i < nsize; i++)
        printf("%d ", nums[i]);
}

static int ggcds[14][14]; // problem statement says maximum 14 numbers

// old:
static int _gcd(int n1, int n2) {
    int min, max, gcd;
    
    if (n1 <= n2) {
        min = n1;
        max = n2;
    } else {
        max = n1;
        min = n2;
    }
    
    for (int i = min; i >= 1; i--) {
        if (!(min % i)) {
            if (!(max % i)) {
                gcd = i;
                goto out;
            }
        }
    }
    
out:
    assert(gcd);
    //printf("gcd of %d %d is %d\n", n1, n2, gcd);
    return gcd;
}

// cleaner and faster:
static int gcd(int a, int b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

static int _max_score(int *nums, int nsize, int start_mult) {
    int max_sum = 0, sum, this_sum, sa_ind;
    int *sub_array = NULL;
    
    //printf("_max_score nums: ");
    //dump_array(nums, nsize);
    //printf("\n");
    
    assert(nsize % 2 == 0 && nsize > 0);
    
    if (nsize > 2) {
        sub_array = malloc((nsize - 2) * sizeof(int));
        assert(sub_array);
    }
    
    for (int i = 0; i < nsize; i++) {
        for (int j = i + 1; j < nsize; j++) {
            //printf("looking at %d %d\n", nums[i], nums[j]);
            this_sum = start_mult * gcd(nums[i], nums[j]); // current
            //printf("ggcds[%d][%d] %d (gcd %d)\n", i, j, ggcds[i][j], gcd(nums[i], nums[j]));
            //assert(ggcds[i][j] == gcd(nums[i], nums[j]));
            //this_sum = start_mult * ggcds[i][j]; // new; doesn't work because of use of subarrays
            if (nsize > 2) {
                sa_ind = 0;
                for (int k = 0; k < nsize; k++) {
                    if (k == i || k == j)
                        continue;
                    sub_array[sa_ind++] = nums[k];
                }
                sum =  this_sum + _max_score(sub_array, nsize - 2, start_mult + 1);
            } else {
                sum = this_sum;
            }
            if (sum > max_sum)
                max_sum = sum;
        }
    }
    
    free(sub_array);
    
    //printf("returning %d\n", max_sum);
    
    return max_sum;
}

int maxScore(int* nums, int numsSize){
    // pre-compute gcds
    assert(numsSize <= 14);
    for (int i = 0; i < numsSize; i++) {
        for (int j = i + 1; j < numsSize; j++) {
            ggcds[i][j] = gcd(nums[i], nums[j]);
            //printf("ggcds[%d][%d] %d (gcd %d)\n", i, j, ggcds[i][j], gcd(nums[i], nums[j]));
        }
    }
    return _max_score(nums, numsSize, 1);
}
