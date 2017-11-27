#include "parse.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>

int32_t parse_path_str(char * restrict pathstr, char ** restrict pathbuf) {
    char *token = strtok(pathstr, "/");
    int32_t ret_code;

    if (pathstr == NULL || pathbuf == NULL) {
        return -EINVAL;
    }

    for (ret_code = 0; token != NULL; ++ret_code) {
        pathbuf[ret_code] = malloc(sizeof(char) * BUFSIZ);
        strcpy(pathbuf[ret_code], token);
        
        token = strtok(NULL, "/");
    }

    return ret_code;
} 
