#include <getopt.h>
#include <stdio.h>

#include "on_disk_system.h"

int main(int argc, char **argv) {
    char *path = NULL;
    int ch;
    
    struct option longopts[] = {
        { "path",   required_argument,   NULL,   'p' },
        { NULL,                0,        NULL,    0 }
    };

    while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch (ch) {
            case 'p':
                path = optarg;
                break;
            default:
                printf("usage: %s --path <disk-or-file-path>\n", argv[0]);
                return -1;
        }
    }
    
    if (path == NULL) {
        printf("usage: %s --path <disk-or-file-path>\n", argv[0]);
        return -1;
    }
    
    ods_dump_disk(path);
    
    return 0;
}
