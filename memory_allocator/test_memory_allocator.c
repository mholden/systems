#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdint.h>

#include "memory_allocator.h"
#include "synch.h"
#include "hash_table.h"

typedef struct tmar_data {
    void *p;
    uint8_t data;
    size_t size;
} tmar_data_t;

static unsigned int tmar_hash_fn(void *data) {
    tmar_data_t *tdata = (tmar_data_t *)data;
    return ht_default_hash(&tdata->p, sizeof(tdata->p));
}

static int tmar_compare(void *data1, void *data2) {
    tmar_data_t *td1, *td2;
    
    td1 = (tmar_data_t *)data1;
    td2 = (tmar_data_t *)data2;
    
    if (td1 == td2)
        return 0;
    else
        return -1;
}

static void tmar_destroy_data(void *data) {
    tmar_data_t *tdata = (tmar_data_t *)data;
    free(tdata);
}

static hash_table_ops_t tmar_ht_ops = {
    .hto_hash_fn = tmar_hash_fn,
    .hto_lops = {
        .llo_compare_fn = tmar_compare,
        .llo_destroy_data_fn = tmar_destroy_data,
        .llo_dump_data_fn = NULL
    }
};

static int tmar_ht_cleanup(void *data, void *ctx, bool *stop) {
    tmar_data_t *tdata = (tmar_data_t *)data;
    void *p = tdata->p;
    
    // check data integrity
    for (int j = 0; j < tdata->size; j++)
        assert(*((uint8_t *)p + j) == tdata->data);
    
    ma_free(tdata->p);
    
    return 0;
}

typedef struct tmar_targ {
    thread_t *t;
    int num_ops;
    bool allow_page_allocs;
} tmar_targ_t;

static int tmar_thread_fn(void *arg) {
    tmar_targ_t *targ = (tmar_targ_t *)arg;
    //thread_t *t = targ->t;
    int num_ops, op, nallocs = 0, nfrees = 0, zind;
    size_t size_to_alloc;
    void *p;
    hash_table_t *ht;
    tmar_data_t *tmd;
    uint8_t data;
    mem_chunk_header_t *mch;
    bool allow_page_allocs = targ->allow_page_allocs;
    
    num_ops = targ->num_ops;
    //printf("  tmar_thread_fn: thread %s (num_ops %d)\n", t->t_name, num_ops);
    
    assert(ht = ht_create(&tmar_ht_ops));
    
    for (int i = 0; i < num_ops; i++) {
        op = rand() % 2;
        if (op == 0 || ht_empty(ht)) { // alloc
            
            // get a random size to alloc. distribute allocations between zones evenly
            if (allow_page_allocs)
                zind = rand() % (MA_NUM_ZONES + 1);
            else
                zind = rand() % MA_NUM_ZONES;
            
            size_to_alloc = zind ? ma_zone_size(zind-1) : 0;
            if (zind == MA_NUM_ZONES)
                size_to_alloc += ((rand() % (PA_PAGE_SIZE - size_to_alloc)) + 1);
            else
                size_to_alloc += ((rand() % (ma_zone_size(zind) - size_to_alloc)) + 1);
            
            assert((size_to_alloc > 0) && (size_to_alloc <= PA_PAGE_SIZE));
            
            // do the allocation
            assert(p = ma_alloc(size_to_alloc));
            
            if (!allow_page_allocs || (zind < MA_NUM_ZONES)) {
                mch = (mem_chunk_header_t *)((uint8_t *)p - sizeof(mem_chunk_header_t));
                assert(mch->mch_sz == size_to_alloc);
            }
            
            // put some random data in the memory
            data = rand() % UINT8_MAX;
            memset(p, data, size_to_alloc);
            
            // store it in our hash table so we can check it later
            assert(tmd = malloc(sizeof(tmar_data_t)));
            tmd->p = p;
            tmd->size = size_to_alloc;
            tmd->data = data;
            
            assert(ht_insert(ht, (void *)tmd) == 0);
            
            nallocs++;
        } else { // free
            assert(ht_get_random(ht, (void **)&tmd) == 0);
            p = tmd->p;
            
            // check data integrity
            for (int j = 0; j < tmd->size; j++)
                assert(*((uint8_t *)p + j) == tmd->data);
            
            ma_free(tmd->p);
            assert(ht_remove(ht, tmd) == 0);
            
            nfrees++;
        }
        ma_check();
    }
    
    //printf("  tmar_thread_fn: thread %s did %d allocs %d frees\n", t->t_name, nallocs, nfrees);
    
    // free anything we haven't freed
    assert(ht_iterate(ht, tmar_ht_cleanup, NULL) == 0);
    
    ht_destroy(ht);
    
    ma_check();
    
    return 0;
}

#define TMA_NTHREADS 8
static void test_memory_allocator_random(int num_ops) {
    thread_t *t[TMA_NTHREADS];
    tmar_targ_t targs[TMA_NTHREADS];
    char tname[5];
    
    printf("test_memory_allocator_random (num_ops %d)\n", num_ops);
    
    for (int i = 0; i < TMA_NTHREADS; i++) {
        sprintf(tname, "thr%d", i);
        assert(t[i] = thread_create(tname));
        
        memset(&targs[i], 0, sizeof(tmar_targ_t));
        targs[i].t = t[i];
        targs[i].num_ops = num_ops / TMA_NTHREADS;
        if (i <= 1) // allow 2 threads to do page allocations
            targs[i].allow_page_allocs = true;
        
        assert(thread_start(t[i], tmar_thread_fn, (void *)&targs[i]) == 0);
    }
    
    for (int i = 0; i < TMA_NTHREADS; i++)
        assert(thread_wait(t[i], NULL) == 0);
    
    ma_check();
    
    //printf("test_memory_allocator_random: done waiting on all threads\n");
    
    for (int i = 0; i < TMA_NTHREADS; i++)
        thread_destroy(t[i]);
    
    //ma_dump(false);
    
    ma_teardown();
    
    ma_check();
}

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
        //ma_dump(false);
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
        //ma_dump(false);
    }
    
    assert(ma_nalloced() == nalloced);
    assert(ma_nalloced() == 0);
    
    //ma_dump(false);
    
    ma_teardown();
    
    ma_check();
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
        //ma_dump(false);
    }
    
    for (int i = 0; i < MA_NUM_ZONES; i++) {
        ma_free(p[i]);
        assert(ma_nalloced() == (MA_NUM_ZONES - i - 1));
        ma_check();
        //ma_dump(false);
    }
    
    ma_teardown();
    
    ma_check();
    //ma_dump(false);
    
    return;
}

#define DEFAULT_NUM_OPS 1024

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
    test_memory_allocator_random(num_ops);
    
    return 0;
}
