#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

typedef struct coordinate {
    int c_x;
    int c_y;
} coord_t;

#define RSD_NORTH           0
#define RSD_EAST            1
#define RSD_SOUTH           2
#define RSD_WEST            3
#define RSD_NDIRECTIONS     4

typedef struct robot_state {
    coord_t rs_coord;
    int8_t rs_direction;
} rs_t;

static void rs_change_dir(rs_t *rs, char turn) {
    switch (turn) {
        case 'L':
            rs->rs_direction--;
            if (rs->rs_direction < 0)
                rs->rs_direction = RSD_WEST;
            break;
        case 'R':
            rs->rs_direction = (rs->rs_direction + 1) % RSD_NDIRECTIONS;
            break;
        default:
            assert(0);
            break;
    }
}

static void rs_move_one(rs_t *rs) {
    switch (rs->rs_direction) {
        case RSD_NORTH:
            rs->rs_coord.c_y++;
            break;
        case RSD_EAST:
            rs->rs_coord.c_x++;
            break;
        case RSD_SOUTH:
            rs->rs_coord.c_y--;
            break;
        case RSD_WEST:
            rs->rs_coord.c_x--;
            break;
        default:
            assert(0);
            break;
    }
}

bool isRobotBounded(char * instructions){
    rs_t rs;
    size_t nins; // number of instructions
    char *ins;
    
    memset(&rs, 0, sizeof(rs_t)); // sets robot to 0,0 facing north
    
    nins = strlen(instructions);
    do {
        ins = instructions;
        for (int i = 0; i < nins; i++) {
            if (*ins == 'G') {
                rs_move_one(&rs);
            } else {
                assert(*ins == 'L' || *ins == 'R');
                rs_change_dir(&rs, *ins);
            }
            ins++;
        }
    } while (rs.rs_direction != RSD_NORTH);
    
    return (rs.rs_coord.c_x == 0 && rs.rs_coord.c_y == 0);
}

bool robot_bounded_in_circle(char * instructions) {
    return isRobotBounded(instructions);
}
