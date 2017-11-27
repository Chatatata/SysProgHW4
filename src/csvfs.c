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
            if (sscanf(pathbuf[1], "%u", &c_code) == 1 && c_code <= 81 && c_code > 0) {
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

            int grep_code = 0;
            char **grep_res = malloc(sizeof(char *) * BUFSIZ);

            grep_code = fgrep_city_by_name("file.csv", pathbuf[1], grep_res);

            if (grep_code < 0) {
                ret_code = grep_code;

                goto bailout;
            } else if (grep_code == 0) {
                ret_code = -ENOENT;

                goto bailout;
            }

            //  We're in NAMES directory
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 3;
            stbuf->st_size = 8;
        } else {
            ret_code == -ENOENT;

            goto bailout;
        }
    } else if (ret_code == 3) {
        if (strcmp(pathbuf[0], "CODES") == 0) {
            if (sscanf(pathbuf[1], "%u", &c_code) == 1 && c_code <= 81) {
                stbuf->st_mode = S_IFREG | 0755;
                stbuf->st_nlink = 4;
                stbuf->st_size = BUFSIZ;
            } else {
                ret_code = -ENOENT;

                goto bailout;
            }
        } else if (strcmp(pathbuf[0], "NAMES") == 0) {
            char **grep_res = malloc(sizeof(char *) * BUFSIZ);
            int grep_code = 0;

            grep_code = fgrep_city_by_name("file.csv", pathbuf[1], grep_res);

            if (grep_code < 0) {
                ret_code = grep_code;

                goto bailout;
            } else if (grep_code == 0) {
                ret_code = -ENOENT;

                goto bailout;
            }

            char **grep_res_2nd = malloc(sizeof(char *) * BUFSIZ);

            grep_code = grep_district_by_name((const char **)grep_res, grep_code, pathbuf[2], grep_res_2nd);

            if (grep_code < 0) {
                ret_code = grep_code;

                goto bailout;
            } else if (grep_code == 0) {
                ret_code = -ENOENT;

                goto bailout;
            }

            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 4;
            stbuf->st_size = BUFSIZ;
        } else {
            ret_code = -ENOENT;

            goto bailout;
        }
    } else if (ret_code == 4 && strcmp(pathbuf[0], "NAMES") == 0) {
        char **grep_res = malloc(sizeof(char *) * BUFSIZ);
        int grep_code = 0;
        
        grep_code = fgrep_city_by_name("file.csv", pathbuf[1], grep_res);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            ret_code = -ENOENT;

            goto bailout;
        }

        debug_print("fgrep returned with %d\n", grep_code);

        char **grep_res_2nd = malloc(sizeof(char *) * BUFSIZ);

        grep_code = grep_district_by_name((const char **)grep_res, grep_code, pathbuf[2], grep_res_2nd);

        debug_print("grep_dist returned with %d\n", grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            ret_code = -ENOENT;

            goto bailout;
        }

        char **grep_res_3rd = malloc(sizeof(char *) * BUFSIZ);

        pathbuf[3][strlen(pathbuf[3]) - 4] = '\0';

        debug_print("pathbuf[3] = %s\n", pathbuf[3]);

        grep_code = grep_neighbor_by_name((const char **)grep_res_2nd, grep_code, pathbuf[3], grep_res_3rd);

        debug_print("grep_neigh returned with %d\n", grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            ret_code = -ENOENT;

            goto bailout;
        }

        stbuf->st_mode = S_IFREG | 0755;
        stbuf->st_nlink = 5;
        stbuf->st_size = BUFSIZ;
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

            debug_print("fgrep result for %d entries:\n", grep_code);

            if (grep_code < 0) {
                ret_code = grep_code;

                goto bailout;
            }

            char **grep_2nd_res = malloc(sizeof(char *) * BUFSIZ);

            debug_print("path params parsed:\n", NULL);

            for (i = 0; i < ret_code; ++i) {
                debug_print("\t%d: \"%s\"\n", i, pathbuf[i]);
            }
            
            debug_print("districts are being normalized...\n", NULL);
            
            grep_code = normalize_entries_for_district((const char **)grep_res, grep_code, grep_2nd_res);

            debug_print("%d entries normalized:\n", grep_code);

            for (i = 0; i < grep_code; ++i) {
                filler(buf, grep_2nd_res[i], NULL, 0);
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
        debug_print("path resolved as /NAMES/Istanbul/Sariyer/Maslak.txt\n", NULL);

        grep_code = fgrep_city_by_name("file.csv", pathbuf[1], grep_res);

        debug_print("fgrep result for %d entries:\n", grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        }

        char **grep_2nd_res = malloc(sizeof(char *) * BUFSIZ);

        debug_print("path params parsed:\n", NULL);

        for (i = 0; i < ret_code; ++i) {
            debug_print("\t%d: \"%s\"\n", i, pathbuf[i]);
        }

        grep_code = grep_district_by_name((const char **)grep_res, grep_code, pathbuf[2], grep_2nd_res);

        debug_print("grep result for %d entries:\n", grep_code);

        char **grep_3rd_res = malloc(sizeof(char *) * BUFSIZ);
        
        debug_print("districts are being normalized...\n", NULL);
        
        grep_code = normalize_entries_for_neighbor((const char **)grep_2nd_res, grep_code, grep_3rd_res);

        debug_print("%d entries normalized:\n", grep_code);

        char last_str[BUFSIZ] = "";

        for (i = 0; i < grep_code; ++i) {
            sprintf(last_str, "%s.txt", grep_3rd_res[i]);
            filler(buf, last_str, NULL, 0);
        }

        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);

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

    ret_code = parse_path_str(pathcpy, &pathbuf);

    if (ret_code < 0) {
        return -ENOENT;
    } else if (ret_code == 3) {
        return strcmp(pathbuf[0], "CODES") == 0 ? 0 : -ENOENT; 
    } else if (ret_code == 4 && strcmp(pathbuf[0], "NAMES") == 0) {
        return 0; 
    } else {
        return -ENOENT;
    }
}

static int csvfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void)fi;
    int32_t ret_code = 0L;
    size_t buf_len = 0L;
    int32_t grep_code = 0L;
    int32_t zip_code = 0L;
    char **grep_res = malloc(sizeof(char *) * BUFSIZ);
    char **grep_res_2nd = malloc(sizeof(char *) * BUFSIZ);
    char **grep_res_3rd = malloc(sizeof(char *) * BUFSIZ);
    char *result_buf = malloc(sizeof(char) * BUFSIZ);
    char *pathcpy = malloc(sizeof(char) * BUFSIZ);
    char **pathbuf = malloc(sizeof(char *) * BUFSIZ);
    char *stream_line = malloc(sizeof(char *) * BUFSIZ);
    char *stream_line_raw = malloc(sizeof(char *) * BUFSIZ);

    debug_print("read requested for path: %s\n", path);

    strcpy(pathcpy, path);

    debug_print("path copied as %s\n", pathcpy);
    
    ret_code = parse_path_str(pathcpy, pathbuf);

    debug_print("path parse completed: %d params\n", ret_code);

    if (ret_code < 0) {
        debug_print("root dir requested to open, sizzling ENOENT\n", NULL);
        ret_code = -ENOENT;

        goto bailout;
    } else if (ret_code == 3 && strcmp(pathbuf[0], "CODES") == 0) {
        if (sscanf(pathbuf[1], "%d", &zip_code) != 1 && zip_code <= 0 && zip_code > 81) {
            ret_code = -ENOENT;

            goto bailout;
        }

        grep_code = fgrep_city_by_code("file.csv", zip_code, grep_res);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        if (strlen(pathbuf[2]) > 5) {
            pathbuf[2][strlen(pathbuf[2]) - 4] = '\0';
        } else {
            goto err_noent;
        }

        if (sscanf(pathbuf[2], "%d", &zip_code) != 1) {
            goto err_noent;
        }

        grep_code = grep_district_by_code((const char **)grep_res, grep_code, zip_code, grep_res_2nd);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        strcpy(stream_line_raw, grep_res_2nd[0]);

        goto stream;
    } else if (ret_code == 4 && strcmp(pathbuf[0], "NAMES") == 0) {
        grep_code = fgrep_city_by_name("file.csv", pathbuf[1], grep_res);

        debug_print("fgrep with city: %s:%d\n", pathbuf[1], grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        grep_code = grep_district_by_name((const char **)grep_res, grep_code, pathbuf[2], grep_res_2nd);

        debug_print("grep with dist: %s:%d\n", pathbuf[2], grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        pathbuf[3][strlen(pathbuf[3]) - 4] = '\0';

        grep_code = grep_neighbor_by_name((const char **)grep_res_2nd, grep_code, pathbuf[3], grep_res_3rd);

        debug_print("grep with neigh: %s:%d\n", pathbuf[3], grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        strcpy(stream_line_raw, grep_res_3rd[0]);

        goto stream;
    } else {
        debug_print("non-file requested to open, sizzling ENOENT\n", NULL);
        
        goto err_noent;
    }

err_noent:
    ret_code = -ENOENT;

    goto bailout;

stream:
    debug_print("stream subroutine started with raw line: %s", stream_line_raw);
    dump_entry(stream_line_raw, stream_line);

    debug_print("finalizing by stream with line %s", stream_line);
    buf_len = strlen(stream_line);

    debug_print("found buflen for stream_line: %zu\n", buf_len);

    if (offset < buf_len) {
        debug_print("offset needs more buf, %lu:%zu\n", offset, buf_len);

        if (offset + size > buf_len) {
            size = buf_len - offset;

            debug_print("offset + size > buf_len, shrinking size: %zu\n", size);
        }

        debug_print("copying block to buf with size: %zu\n", size);

        memcpy(buf, stream_line + offset, size);
    } else {
        debug_print("requested for copy block but no remaining\n", NULL);
        size = 0;
    }

    ret_code = size;

bailout:
    free(grep_res);
    free(grep_res_2nd);
    free(grep_res_3rd);
    free(result_buf);
    free(pathcpy);
    free(pathbuf);
    free(stream_line);
    free(stream_line_raw);

    return ret_code;
}

static int32_t csvfs_unlink(const char *path) {
    int32_t ret_code = 0L;
    size_t buf_len = 0L;
    int32_t grep_code = 0L;
    int32_t total_count = 0L;
    int32_t zip_code = 0L;
    int32_t i = 0L, b = 0L;
    char **file_res = malloc(sizeof(char *) * 40000);
    char **grep_res = malloc(sizeof(char *) * BUFSIZ);
    char **grep_res_2nd = malloc(sizeof(char *) * BUFSIZ);
    char **grep_res_3rd = malloc(sizeof(char *) * BUFSIZ);
    char *result_buf = malloc(sizeof(char) * BUFSIZ);
    char *pathcpy = malloc(sizeof(char) * BUFSIZ);
    char **pathbuf = malloc(sizeof(char *) * BUFSIZ);
    char *stream_line = malloc(sizeof(char *) * BUFSIZ);
    char *stream_line_raw = malloc(sizeof(char *) * BUFSIZ);

    debug_print("read requested for path: %s\n", path);

    strcpy(pathcpy, path);

    debug_print("path copied as %s\n", pathcpy);
    
    ret_code = parse_path_str(pathcpy, pathbuf);

    debug_print("path parse completed: %d params\n", ret_code);

    if (ret_code == 3 && strcmp(pathbuf[0], "CODES") == 0) {
        if (sscanf(pathbuf[1], "%d", &zip_code) != 1 && zip_code <= 0 && zip_code > 81) {
            ret_code = -ENOENT;

            goto bailout;
        }

        grep_code = fgrep_city_by_code("file.csv", zip_code, grep_res);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        if (strlen(pathbuf[2]) > 5) {
            pathbuf[2][strlen(pathbuf[2]) - 4] = '\0';
        } else {
            goto err_noent;
        }

        if (sscanf(pathbuf[2], "%d", &zip_code) != 1) {
            goto err_noent;
        }

        grep_code = grep_district_by_code((const char **)grep_res, grep_code, zip_code, grep_res_2nd);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        goto persist_for_all;
    } else if (ret_code == 4 && strcmp(pathbuf[0], "NAMES") == 0) {
        grep_code = fgrep_city_by_name("file.csv", pathbuf[1], grep_res);

        debug_print("fgrep with city: %s:%d\n", pathbuf[1], grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        grep_code = grep_district_by_name((const char **)grep_res, grep_code, pathbuf[2], grep_res_2nd);

        debug_print("grep with dist: %s:%d\n", pathbuf[2], grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        pathbuf[3][strlen(pathbuf[3]) - 4] = '\0';

        grep_code = grep_neighbor_by_name((const char **)grep_res_2nd, grep_code, pathbuf[3], grep_res_3rd);

        debug_print("grep with neigh: %s:%d\n", pathbuf[3], grep_code);

        if (grep_code < 0) {
            ret_code = grep_code;

            goto bailout;
        } else if (grep_code == 0) {
            goto err_noent;
        }

        strcpy(stream_line_raw, grep_res_3rd[0]);
    } else {
err_noent:
        ret_code = -ENOENT;
        
        goto bailout;
    }

persist_for_one:
    total_count = read_file("file.csv", file_res);

    if (total_count < 0) {
        debug_print("file could not be read, EIO\n", NULL);

        ret_code = -EIO;

        goto bailout;
    } else if (total_count == 0) {
        debug_print("file read but nothing found, EIO\n", NULL);

        ret_code = -EIO;

        goto bailout;
    }

    for (i = 0; i < total_count; ++i) {
        if (strcmp(file_res[i], stream_line_raw) == 0) {
            debug_print("found on delete, inserting x\n", NULL);

            file_res[i][0] = 'x';
            break;
        }
    }

    ret_code = write_file("file.csv", file_res, total_count);

    if (ret_code != total_count - 1) {
        debug_print("unexpected write count on write, expected %d found %d\n", total_count - 1, ret_code);

        ret_code = -EIO;

        goto bailout;
    }

    debug_print("written %d lines to file successfully\n", ret_code);

    ret_code = 0;

    goto bailout;

persist_for_all:
    total_count = read_file("file.csv", file_res);

    if (total_count < 0) {
        debug_print("file could not be read, EIO\n", NULL);

        ret_code = -EIO;

        goto bailout;
    } else if (total_count == 0) {
        debug_print("file read but nothing found, EIO\n", NULL);

        ret_code = -EIO;

        goto bailout;
    }

    for (i = 0; i < total_count; ++i) {
        for (b = 0; b < grep_code; ++b) {
            if (strcmp(file_res[i], grep_res_2nd[b]) == 0) {
                debug_print("found on delete, inserting x\n", NULL);

                file_res[i][0] = 'x';
                goto bump;
            }
        }
bump:
        /* null */;
    }

    ret_code = write_file("file.csv", file_res, total_count);

    if (ret_code >= total_count) {
        debug_print("unexpected write count on write, expected %d found %d\n", total_count - 1, ret_code);

        ret_code = -EIO;

        goto bailout;
    }

    debug_print("written %d lines to file successfully\n", ret_code);

    ret_code = 0;

    goto bailout;


bailout:
    free(file_res);
    free(grep_res);
    free(grep_res_2nd);
    free(grep_res_3rd);
    free(result_buf);
    free(pathcpy);
    free(pathbuf);
    free(stream_line);
    free(stream_line_raw);

    return ret_code;
}

static struct fuse_operations csvfs_oper = {
    .getattr = csvfs_getattr,
    .readdir = csvfs_readdir,
    .open = csvfs_open,
    .read = csvfs_read,
    .unlink = csvfs_unlink,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &csvfs_oper, NULL);
}

