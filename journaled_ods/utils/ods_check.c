#include <getopt.h>
#include <stdio.h>
#include <errno.h>

#include "on_disk_system.h"

int main(int argc, char **argv) {
    char *path = NULL;
    int ch;
    int err;
    
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
    
    err = ods_check_disk(path);
    if (err)
        goto error_out;
    
    printf("ods_check: ods is OK\n");
    
    return 0;
    
error_out:
    if (err == EILSEQ)
        printf("ods_check: ods is CORRUPT\n");
    
    return err;
}

