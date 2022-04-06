#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>

#include "memory_allocator.h"

// allocate enough from each zone to require the use of more than 1 page
static void test_memory_allocator2(void) {
    void **p[MA_NUM_ZONES];
    size_t mem_chunk_size, alloc_size, num_to_alloc, nalloced = 0;
    mem_chunk_header_t *mch;
    
    printf("test_memory_allocator2\n");
    
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        mem_chunk_size = 1 << (MA_SMALLEST_ZONE_ORDER + i);
        alloc_size = mem_chunk_size - sizeof(mem_chunk_header_t);
        num_to_alloc = (PA_PAGE_SIZE / mem_chunk_size) * 2;
        p[i] = malloc(sizeof(void *) * num_to_alloc);
        memset(p[i], 0, sizeof(void *) * num_to_alloc);
        for (int j = 0; j < num_to_alloc; j++) {
            assert(p[i][j] = ma_alloc(alloc_size));
            nalloced++;
            mch = (mem_chunk_header_t *)((uint8_t *)p[i][j] - sizeof(mem_chunk_header_t));
            assert(mch->mch_sz == alloc_size);
            ma_check();
        }
        //ma_dump();
    }
    
    assert(ma_nalloced() == nalloced);
    
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        mem_chunk_size = 1 << (MA_SMALLEST_ZONE_ORDER + i);
        alloc_size = mem_chunk_size - sizeof(mem_chunk_header_t);
        num_to_alloc = (PA_PAGE_SIZE / mem_chunk_size) * 2;
        for (int j = 0; j < num_to_alloc; j++) {
            ma_free(p[i][j]);
            nalloced--;
            ma_check();
        }
        free(p[i]);
        //ma_dump();
    }
    
    assert(ma_nalloced() == nalloced);
    assert(ma_nalloced() == 0);
    
    ma_teardown();
    
    ma_check();
    //ma_dump();
}

// allocate and free from each zone
static void test_memory_allocator1(void) {
    void *p[MA_NUM_ZONES];
    size_t alloc_size;
    mem_chunk_header_t *mch;
    
    printf("test_memory_allocator1\n");
    
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        alloc_size = (1 << (MA_SMALLEST_ZONE_ORDER + i)) - sizeof(mem_chunk_header_t);
        assert(p[i] = ma_alloc(alloc_size));
        assert(ma_nalloced() == i + 1);
        mch = (mem_chunk_header_t *)((uint8_t *)p[i] - sizeof(mem_chunk_header_t));
        assert(mch->mch_sz == alloc_size);
        ma_check();
        //ma_dump();
    }
    
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        ma_free(p[i]);
        assert(ma_nalloced() == (MA_NUM_ZONES - i - 1));
        ma_check();
        //ma_dump();
    }
    
    ma_teardown();
    
    ma_check();
    //ma_dump();
    
    return;
}

#define DEFAULT_NUM_OPS 8 * 1024

int main(int argc, char **argv) {
    unsigned int seed = 0;
    int ch, num_ops = DEFAULT_NUM_OPS;
    
    struct option longopts[] = {
        { "num",    required_argument,   NULL,   'n' },
        { "seed",   required_argument,   NULL,   's' },
        { NULL,                0,        NULL,    0 }
    };

    while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch (ch) {
            case 'n':
                num_ops = (int)strtol(optarg, NULL, 10);
                break;
            case 's':
                seed = (unsigned int)strtol(optarg, NULL, 10);
                break;
            default:
                printf("usage: %s [--num <num-elements>] [--seed <seed>]\n", argv[0]);
                return -1;
        }
    }
    
    if (!seed) {
        seed = (unsigned int)time(NULL);
        srand(seed);
    } else
        srand(seed);
    
    printf("seed %u\n", seed);
    
    test_memory_allocator1();
    test_memory_allocator2();
    
    return 0;
}
