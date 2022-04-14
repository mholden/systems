#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "on_disk_system.h"

char *path = NULL;
ods_t *ods = NULL;

// called directly from cmd_table
static int exercise_startup(int argc, char **argv);
static int exercise_shutdown(int argc, char **argv);

struct command {
    const char *name;
    int (*func)(int, char **);
};

static struct command cmd_table[] = {
    {"startup", exercise_startup},
    {"shutdown", exercise_shutdown},
    {NULL, NULL}
};

#define DOUBLE_QUOTE '"'
#define SINGLE_QUOTE '\''
#define BACK_SLASH   '\\'

static char **
build_argv(char *str, int *argc)
{
    int table_size = 16, _argc;
    char **argv;
    
    if (argc == NULL)
        argc = &_argc;
    
    *argc = 0;
    argv = (char **)calloc(table_size, sizeof(char *));
    
    if (argv == NULL)
        return NULL;
    
    while (*str) {
        // skip intervening white space //
        while (*str != '\0' && (*str == ' ' || *str == '\t' || *str == '\n'))
            str++;
        
        if (*str == '\0')
            break;
        
        if (*str == DOUBLE_QUOTE) {
            argv[*argc] = ++str;
            while (*str && *str != DOUBLE_QUOTE) {
                if (*str == BACK_SLASH)
                    memmove(str, str + 1, strlen(str+1)+1); // copy everything down //
                str++;
            }
        } else if (*str == SINGLE_QUOTE) {
            argv[*argc] = ++str;
            while (*str && *str != SINGLE_QUOTE) {
                if (*str == BACK_SLASH)
                    memmove(str, str + 1, strlen(str+1)+1); // copy everything down //
                str++;
            }
        } else {
            argv[*argc] = str;
            while (*str && *str != ' ' && *str != '\t' && *str != '\n') {
                if (*str == BACK_SLASH)
                    memmove(str, str + 1, strlen(str+1)+1); // copy everything down //
                str++;
            }
        }
        
        if (*str != '\0')
            *str++ = '\0'; // chop the string //
        
        *argc = *argc + 1;
        if (*argc >= table_size - 1) {
            char **nargv;
            
            table_size = table_size * 2;
            nargv = (char **)calloc(table_size, sizeof(char *));
            
            if (nargv == NULL) { // drats! failure. //
                free(argv);
                return NULL;
            }
            
            memcpy(nargv, argv, (*argc) * sizeof(char *));
            free(argv);
            argv = nargv;
        }
    }
    
    return argv;
}

static int exercise_startup(int argc, char **argv) {
    int err;
    
    if (ods) {
        printf("exercise_startup: ods already started\n");
        err = EBUSY;
        goto error_out;
    }
    
    err = ods_startup(path, &ods);
    if (err) {
        printf("exercise_startup: ods_startup failed: %s\n", strerror(err));
        goto error_out;
    }
    
    return 0;
    
error_out:
    return err;
}

static int exercise_shutdown(int argc, char **argv) {
    int err;
    
    if (ods == NULL) {
        printf("exercise_shutdown: ods isn't started\n");
        err = ENOENT;
        goto error_out;
    }
    
    err = ods_shutdown(ods);
    if (err) {
        printf("exercise_shutdown: ods_shutdown failed: %s\n", strerror(err));
        goto error_out;
    }
    ods = NULL;
    
    return 0;
    
error_out:
    return err;
}

int main(int argc, char **argv) {
    char in[256] = { 0 };
    int _argc;
    char **_argv = NULL;
    struct command *cmd;
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
    
    while (1) {
        printf(">>");
        fgets(in, sizeof(in), stdin);
        
        _argv = build_argv(in, &_argc);
        if (_argv == NULL || _argc == 0)
            continue;
        
        if (strcmp(_argv[0], "quit") == 0)
            break;
        
        int i;
        for (i=0, cmd = &cmd_table[i]; cmd->name; cmd = &cmd_table[++i]) {
            if (strcmp(cmd->name, _argv[0]) == 0) {
                cmd->func(_argc, _argv);
                break;
            }
        }
        
        if (!cmd->name) {
            printf("unknown command: %s\n", _argv[0]);
            printf("known commands:\n");
            for (i=0, cmd = &cmd_table[i]; cmd->name; cmd = &cmd_table[++i]) {
                printf("%s\n", cmd->name);
            }
        }
        
        
        free(_argv);
    }
    
    return 0;
}
