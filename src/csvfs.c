#define FUSE_USE_VERSION 26
#define _POSIX_C_SOURCE 199309
#define _BSD_SOURCE

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include "csvfs.h"
#include "parse.h"
#include "query.h"

static const char *csvfs_str = "Hello, world!\n";
static const char *csvfs_codes_path = "/CODES";
static const char *csvfs_names_path = "/NAMES";


static int csvfs_getattr(const char *path, struct stat *stbuf) {
    int ret_code = 0;
    char *pathcpy;
    char **pathbuf;
    int c_code = 0;

    debug_print("getattr requested for path: \"%s\"\n", path);

    memset(stbuf, 0, sizeof(struct stat));

    pathcpy = malloc(sizeof(char) * BUFSIZ);
    strcpy(pathcpy, path);

    pathbuf = malloc(sizeof(char *) * BUFSIZ);

    ret_code = parse_path_str(pathcpy, pathbuf);

    if (ret_code < 0) {
        debug_print("path %s could not be resolved\n", path);

        goto bailout;
    }

    debug_print("path %s parsed for %d params\n", path, ret_code);

    if (ret_code == 0) {
        //  This is the case where root is queried
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        stbuf->st_size = 8;
    } else if (ret_code == 1) {
        if (strcmp(pathbuf[0], "CODES") == 0) {
            //  We're querying for codes
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 1;
            stbuf->st_size = 8;
        } else if (strcmp(pathbuf[0], "NAMES") == 0) {
            //  We're querying for names
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 1;
            stbuf->st_size = 8;
        } else {
            ret_code = -ENOENT;

            goto bailout;
        }
    } else if (ret_code == 2) {
        if (strcmp(pathbuf[0], "CODES") == 0) {
            if (sscanf(pathbuf[1], "%u.txt", &c_code) == 1) {
                debug_print("path param parsed as %u\n", c_code);

                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 3;
                stbuf->st_size = BUFSIZ;
            } else {
                ret_code = -ENOENT;

                goto bailout;
            }
        } else if (strcmp(pathbuf[0], "NAMES") == 0) {
            debug_print("path string parsed as %s\n", pathbuf[1]);

            //  We're in NAMES directory
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 3;
            stbuf->st_size = 8;
        } else {
            ret_code == -ENOENT;

            goto bailout;
        }
    } else {
        ret_code = -ENOENT;

        goto bailout;
    }
bailout:
    free(pathcpy);
    free(pathbuf);

    return ret_code > 0 ? 0 : (-ret_code);
}

static int csvfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    int ret_code;
    int grep_code;
    char *pathcpy;
    char **grep_res;
    char *nr_conv;
    size_t i;
    uint32_t zip_codes[40000];
    (void)offset;
    (void)fi;

    debug_print("readdir requested for path: %s\n", path);

    pathcpy = malloc(sizeof(char) * strlen(path));
    strcpy(pathcpy, path);

    char **pathbuf = malloc(sizeof(char *) * BUFSIZ);
    ret_code = parse_path_str(pathcpy, pathbuf);

    grep_res = malloc(sizeof(char *) * 40000);

    if (ret_code < 0) {
        goto bailout;
    }

    debug_print("path string prs cnt: %u\n", ret_code);

    if (ret_code == 0) {
        debug_print("path resolved as /\n", NULL);

        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, csvfs_codes_path + 1, NULL, 0);
        filler(buf, csvfs_names_path + 1, NULL, 0);
    } else if (ret_code == 1) {
        if (strcmp(pathbuf[0], "CODES") == 0) {
            //  CODES/34
            debug_print("path resolved as /CODES\n", NULL);

            filler(buf, ".", NULL, 0);
            filler(buf, "..", NULL, 0);

            for (size_t i = 1; i <= 81; ++i) {
                nr_conv = malloc(sizeof(char) * BUFSIZ);

                sprintf(nr_conv, "%u", i);
                
                filler(buf, nr_conv, NULL, 0);

                free(nr_conv);
            }
        } else if (strcmp(pathbuf[0], "NAMES") == 0) {
            //  NAMES/Istanbul
            debug_print("path resolved as /NAMES\n", NULL);

            filler(buf, ".", NULL, 0);
            filler(buf, "..", NULL, 0);
            
            grep_code = read_file("file.csv", grep_res);

            if (grep_code < 0) {
                ret_code = grep_code;

                goto bailout;
            }

            char **grep_2nd_res = malloc(sizeof(char *) * BUFSIZ);

            grep_code = normalize_entries_for_city((const char ** restrict)grep_res, grep_code, grep_2nd_res);

            for (size_t a = 0; a < grep_code; ++a) {
                filler(buf, grep_2nd_res[a], NULL, 0);
            }

            free(grep_2nd_res);
        } else {
            debug_print("unknown path /*\n", NULL);
            ret_code = -ENOENT;

            goto bailout;
        }
    } else if (ret_code == 2) {
        if (strcmp(pathbuf[0], "CODES") == 0 && sscanf(pathbuf[1], "%u", &i) == 1) {
            //  CODES/34/34398.txt
            debug_print("path resolved as /CODES/34/34398.txt\n", NULL);

            grep_code = fgrep_city_by_code("file.csv", i, grep_res);

            if (grep_code < 0) {
                ret_code = grep_code;

                goto bailout;
            }

            grep_code = normalize_entries_for_zip((const char ** restrict)grep_res, grep_code, (uint32_t * restrict)&zip_codes);

            if (grep_code < 0) {
                ret_code = grep_code;

                goto bailout;
            }

            nr_conv = malloc(sizeof(char) * BUFSIZ);

            for (i = 0; i < grep_code; ++i) {
                sprintf(nr_conv, "%u.txt", zip_codes[i]);

                filler(buf, nr_conv, NULL, 0);

                memset(nr_conv, 0, sizeof(char) * BUFSIZ);
            }

            free(nr_conv);

            filler(buf, ".", NULL, 0);
            filler(buf, "..", NULL, 0);
        } else if (strcmp(pathbuf[0], "NAMES") == 0) {
            //  NAMES/Istanbul/Sariyer
            debug_print("path resolved as /NAMES/Istanbul/Sariyer\n", NULL);

            grep_code = fgrep_city_by_name("file.csv", pathbuf[1], grep_res);

            if (grep_code < 0) {
                ret_code = grep_code;

                goto bailout;
            }

            char **grep_2nd_res = malloc(sizeof(char *) * BUFSIZ);
            
            grep_code = grep_district_by_name((const char **)grep_res, grep_code, pathbuf[2], grep_2nd_res);

            char **grep_3rd_res = malloc(sizeof(char *) * BUFSIZ);

            grep_code = normalize_entries_for_district((const char **)grep_2nd_res, grep_code, grep_3rd_res);

            for (i = 0; i < grep_code; ++i) {
                debug_print("%s\n", grep_3rd_res[i]);
                filler(buf, grep_3rd_res[i], NULL, 0);
            }

            filler(buf, ".", NULL, 0);
            filler(buf, "..", NULL, 0);
        } else {
            debug_print("unknown path /*/*n", NULL);
            ret_code = -ENOENT;

            goto bailout;
        }
    } else if (ret_code == 3) {
        //  NAMES/Istanbul/Sariyer/Maslak.txt
    } else {
        ret_code = -ENOENT;

        goto bailout;
    }
bailout:
    free(pathcpy);
    free(pathbuf);
    free(grep_res);

    return 0;
}

static int csvfs_open(const char *path, struct fuse_file_info *fi) {
    debug_print("open requested for path: %s\n", path);

    char pathcpy[BUFSIZ];
    strcpy(&pathcpy, path);

    char *pathbuf[BUFSIZ];
    
    int ret_code = 0;

    ret_code = parse_path_str(&pathcpy, &pathbuf);

    if (ret_code < 0) {
        return -ENOENT;
    } else if (ret_code == 3) {
        return strcmp(&pathbuf[0], "CODES") == 0 ? 0 : -ENOENT; 
    } else if (ret_code == 4) {
        return strcmp(&pathbuf[0], "NAMES") == 0 ? 0 : -ENOENT;
    } else {
        return -ENOENT;
    }
}

static int csvfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    size_t len;
    (void)fi;

    debug_print("read requested for path: %s\n", path);

    char pathcpy[BUFSIZ];
    strcpy(&pathcpy, path);

    char *pathbuf[BUFSIZ];
    
    int ret_code = 0;

    ret_code = parse_path_str(&pathcpy, &pathbuf);

    if (ret_code < 0) {
        return -ENOENT;
    } else if (ret_code == 3) {
        
    } else if (ret_code == 4) {
        return strcmp(&pathbuf[0], "NAMES") == 0 ? 0 : -ENOENT;
    } else {
        return -ENOENT;
    }

    return size;
}

static struct fuse_operations csvfs_oper = {
    .getattr = csvfs_getattr,
    .readdir = csvfs_readdir,
    .open = csvfs_open,
    .read = csvfs_read,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &csvfs_oper, NULL);
}

