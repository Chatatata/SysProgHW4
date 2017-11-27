#include "query.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

int32_t read_file(const char * restrict file_name, char ** restrict out_buf) {
    int32_t ret_code = 0UL;
    FILE *filep;
    char *buf;
    size_t str_len;

    if (file_name == NULL || out_buf == NULL) {
        return -EINVAL;
    }

    if ((filep = fopen(file_name, "r")) == NULL) {
        return -ENOENT;
    }

    if ((buf = malloc(sizeof(char) * BUFSIZ)) == NULL) {
        return -ENOMEM;
    }

    while (fgets(buf, BUFSIZ, filep) != NULL) {
        //  FIXME: string length should not be constant
        str_len = 70;

        out_buf[ret_code] = malloc(sizeof(char) * str_len);
        strcpy(out_buf[ret_code], buf);

        ret_code += 1;

        memset(buf, 0, sizeof(char) * BUFSIZ);
    }

    free(buf);

    return ret_code;
}

int32_t write_file(const char * restrict file_name, char ** restrict buf, uint32_t len) {
    int32_t i;
    int32_t ret_code = 0L;
    FILE *filep;

    if ((filep = fopen(file_name, "w")) == NULL) {
        return -ENOENT;
    }

    for (i = 0; i < len; ++i) {
        if (buf[i][0] == 'x') {
            continue;
        }

        fprintf(filep, "%s", buf[i]);
        ret_code += 1;
    }

    fclose(filep);

    return ret_code;
}

int32_t grep_city_by_name(const char ** restrict buf, uint32_t len, const char * restrict cand_name, char ** restrict out_buf) {
    size_t i;
    size_t str_len;
    char *c_name = malloc(sizeof(char) * BUFSIZ);
    char *d_name = malloc(sizeof(char) * BUFSIZ);
    char *n_name = malloc(sizeof(char) * BUFSIZ);
    char *lat = malloc(sizeof(char) * BUFSIZ);
    char *lon = malloc(sizeof(char) * BUFSIZ);
    uint32_t zip = 0UL;
    int32_t ret_code = 0UL;
    
    for (i = 0; i < len; i++) {
        if (sscanf(buf[i], "%u\t%s\t%s\t%s\t%s\t%s", &zip, n_name, c_name, d_name, lat, lon) != 6) {
            ret_code = -EINVAL;

            debug_print("line reader cannot resolve line %d, read %d fields instead\n", i, ret_code);

            goto bailout;
        }

        if (strcmp(cand_name, c_name) == 0) {
            //  FIXME: string length should not be constant
            str_len = 100;

            out_buf[ret_code] = malloc(sizeof(char) * str_len);
            strcpy(out_buf[ret_code], buf[i]);

            ret_code += 1;
        }
    }

bailout:
    free(c_name);
    free(d_name);
    free(n_name);
    free(lat);
    free(lon);

    return ret_code;
}

int32_t fgrep_city_by_name(const char * restrict file_name, const char * restrict c_name, char ** restrict out_buf) {
    char **entries;
    int32_t ret_code = 0UL;

    if (file_name == NULL || c_name == NULL || out_buf == NULL) {
        return -EINVAL;
    }

    entries = malloc(sizeof(char *) * 40000);
    memset(entries, 0, sizeof(char *) * 40000);

    ret_code = read_file(file_name, entries);

    if (ret_code < 0) {
        debug_print("read_file exit with error code (%d)\n", ret_code);

        goto bailout;
    }

    debug_print("read %d lines from file %s\n", ret_code, file_name);

    ret_code = grep_city_by_name((const char ** restrict)entries, (uint32_t)ret_code, c_name, out_buf); 
bailout:
    free(entries);

    return ret_code;
}

int32_t grep_city_by_code(const char ** restrict buf, uint32_t len, const uint8_t code, char ** restrict out_buf) {
    size_t i;
    size_t str_len;
    char *c_name = malloc(sizeof(char) * BUFSIZ);
    char *d_name = malloc(sizeof(char) * BUFSIZ);
    char *n_name = malloc(sizeof(char) * BUFSIZ);
    char *lat = malloc(sizeof(char) * BUFSIZ);
    char *lon = malloc(sizeof(char) * BUFSIZ);
    uint32_t zip = 0UL;
    int32_t ret_code = 0UL;
    
    for (i = 0; i < len; i++) {
        if (sscanf(buf[i], "%u\t%s\t%s\t%s\t%s\t%s", &zip, n_name, c_name, d_name, lat, lon) != 6) {
            ret_code = -EINVAL;

            debug_print("line reader cannot resolve line %d, read %d fields instead\n", i, ret_code);

            goto bailout;
        }

        if (code == zip / 1000) {
            //  FIXME: string length should not be constant
            str_len = 100;

            out_buf[ret_code] = malloc(sizeof(char) * str_len);
            strcpy(out_buf[ret_code], buf[i]);

            ret_code += 1;
        }
    }

bailout:
    free(c_name);
    free(d_name);
    free(n_name);
    free(lat);
    free(lon);

    return ret_code;
}

int32_t fgrep_city_by_code(const char * restrict file_name, const uint8_t code, char ** restrict out_buf) {
    char **entries;
    int32_t ret_code = 0UL;

    if (file_name == NULL || out_buf == NULL) {
        return -EINVAL;
    }

    entries = malloc(sizeof(char *) * 40000);
    memset(entries, 0, sizeof(char *) * 40000);

    ret_code = read_file(file_name, entries);

    if (ret_code < 0) {
        debug_print("read_file exit with error code (%d)\n", ret_code);

        goto bailout;
    }

    debug_print("read %d lines from file %s\n", ret_code, file_name);

    ret_code = grep_city_by_code((const char ** restrict)entries, (uint32_t)ret_code, code, out_buf); 
bailout:
    free(entries);

    return ret_code;
}

int32_t grep_district_by_name(const char ** restrict buf, uint32_t len, const char * restrict cand_name, char ** restrict out_buf) {
    size_t i;
    size_t str_len;
    char *c_name = malloc(sizeof(char) * BUFSIZ);
    char *d_name = malloc(sizeof(char) * BUFSIZ);
    char *n_name = malloc(sizeof(char) * BUFSIZ);
    char *lat = malloc(sizeof(char) * BUFSIZ);
    char *lon = malloc(sizeof(char) * BUFSIZ);
    uint32_t zip = 0UL;
    int32_t ret_code = 0UL;
    
    for (i = 0; i < len; i++) {
        if (sscanf(buf[i], "%u\t%s\t%s\t%s\t%s\t%s", &zip, n_name, c_name, d_name, lat, lon) != 6) {
            ret_code = -EINVAL;

            debug_print("line reader cannot resolve line %d, read %d fields instead\n", i, ret_code);

            goto bailout;
        }

        if (strcmp(cand_name, d_name) == 0) {
            //  FIXME: string length should not be constant
            str_len = 100;

            out_buf[ret_code] = malloc(sizeof(char) * str_len);
            strcpy(out_buf[ret_code], buf[i]);

            ret_code += 1;
        }
    }

bailout:
    free(c_name);
    free(d_name);
    free(n_name);
    free(lat);
    free(lon);

    return ret_code;
}

int32_t fgrep_district_by_name(const char * restrict file_name, const char * restrict d_name, char ** restrict out_buf) {
    char **entries;
    int32_t ret_code = 0UL;

    if (file_name == NULL || d_name == NULL || out_buf == NULL) {
        return -EINVAL;
    }

    entries = malloc(sizeof(char *) * 40000);
    memset(entries, 0, sizeof(char *) * 40000);

    ret_code = read_file(file_name, entries);

    if (ret_code < 0) {
        debug_print("read_file exit with error code (%d)\n", ret_code);

        goto bailout;
    }

    debug_print("read %d lines from file %s\n", ret_code, file_name);

    ret_code = grep_city_by_name((const char ** restrict)entries, (uint32_t)ret_code, d_name, out_buf); 
bailout:
    free(entries);

    return ret_code;
}

int32_t grep_district_by_code(const char ** restrict buf, uint32_t len, const uint32_t code, char ** restrict out_buf) {
    size_t i;
    size_t str_len;
    char *c_name = malloc(sizeof(char) * BUFSIZ);
    char *d_name = malloc(sizeof(char) * BUFSIZ);
    char *n_name = malloc(sizeof(char) * BUFSIZ);
    char *lat = malloc(sizeof(char) * BUFSIZ);
    char *lon = malloc(sizeof(char) * BUFSIZ);
    uint32_t zip = 0UL;
    int32_t ret_code = 0UL;
    
    for (i = 0; i < len; i++) {
        if (sscanf(buf[i], "%u\t%s\t%s\t%s\t%s\t%s", &zip, n_name, c_name, d_name, lat, lon) != 6) {
            ret_code = -EINVAL;

            debug_print("line reader cannot resolve line %d, read %d fields instead\n", i, ret_code);

            goto bailout;
        }

        if (code == zip) {
            //  FIXME: string length should not be constant
            str_len = 100;

            out_buf[ret_code] = malloc(sizeof(char) * str_len);
            strcpy(out_buf[ret_code], buf[i]);

            ret_code += 1;
        }
    }

bailout:
    free(c_name);
    free(d_name);
    free(n_name);
    free(lat);
    free(lon);

    return ret_code;

}

int32_t grep_neighbor_by_name(const char ** restrict buf, uint32_t len, const char * restrict cand_name, char ** restrict out_buf) {
    size_t i;
    size_t str_len;
    char *c_name = malloc(sizeof(char) * BUFSIZ);
    char *d_name = malloc(sizeof(char) * BUFSIZ);
    char *n_name = malloc(sizeof(char) * BUFSIZ);
    char *lat = malloc(sizeof(char) * BUFSIZ);
    char *lon = malloc(sizeof(char) * BUFSIZ);
    uint32_t zip = 0UL;
    int32_t ret_code = 0UL;
    
    for (i = 0; i < len; i++) {
        if (sscanf(buf[i], "%u\t%s\t%s\t%s\t%s\t%s", &zip, n_name, c_name, d_name, lat, lon) != 6) {
            ret_code = -EINVAL;

            debug_print("line reader cannot resolve line %d, read %d fields instead\n", i, ret_code);

            goto bailout;
        }

        if (strcmp(cand_name, n_name) == 0) {
            //  FIXME: string length should not be constant
            str_len = 100;

            out_buf[ret_code] = malloc(sizeof(char) * str_len);
            strcpy(out_buf[ret_code], buf[i]);

            ret_code += 1;
        }
    }

bailout:
    free(c_name);
    free(d_name);
    free(n_name);
    free(lat);
    free(lon);

    return ret_code;
}

int32_t get_entity_for_entry(const char * restrict entry, enum entity_name_t etty, void * restrict arg) {
    uint32_t zip = 0UL;
    char *c_name = malloc(sizeof(char) * BUFSIZ);
    char *d_name = malloc(sizeof(char) * BUFSIZ);
    char *n_name = malloc(sizeof(char) * BUFSIZ);
    char *lat = malloc(sizeof(char) * BUFSIZ);
    char *lon = malloc(sizeof(char) * BUFSIZ);
    int32_t ret_code = 0UL;

    if (entry == NULL || arg == NULL) {
        ret_code = -EINVAL;

        goto bailout;
    }

    if (sscanf(entry, "%u\t%s\t%s\t%s\t%s\t%s", &zip, n_name, c_name, d_name, lat, lon) != 6) {
        ret_code = -EINVAL;

        goto bailout;
    }

    switch (etty) {
        case etty_zip:
            *((uint32_t *)arg) = zip;
            break;

        case etty_c_name:
            strcpy(arg, c_name);
            break;

        case etty_d_name:
            strcpy(arg, d_name);
            break;

        case etty_n_name:
            strcpy(arg, n_name);
            break;

        case etty_lat:
            strcpy(arg, lat);
            break;

        case etty_lon:
            strcpy(arg, lon);
            break;
    }
bailout:
    free(c_name);
    free(d_name);
    free(n_name);
    free(lat);
    free(lon);

    return ret_code;
}

int32_t normalize_entries_for_zip(const char ** restrict entries, uint32_t len, uint32_t * restrict out_buf) {
    size_t i, b;
    uint32_t extracted_zip;
    int32_t ret_code = 0UL, grep_code = 0UL;

    for (i = 0; i < len; ++i) {
        grep_code = get_entity_for_entry(entries[i], etty_zip, &extracted_zip);

        if (grep_code < 0) {
            debug_print("entity could not be fetched from entry %s, reason: %d\n", entries[i], grep_code);

            return grep_code;
        }

        for (b = 0; b < ret_code; ++b) {
            if (out_buf[b] == extracted_zip) {
                goto pass;
            }
        }

        out_buf[ret_code] = extracted_zip;
        ret_code += 1;
pass: 
        /* pass */;
    }

    return ret_code;
}

int32_t normalize_entries_for_city(const char ** restrict entries, uint32_t len, char ** restrict out_buf) {
    size_t i, b;
    char *extracted_name = malloc(sizeof(char) * BUFSIZ);
    int32_t ret_code = 0UL, grep_code = 0UL;

    for (i = 0; i < len; ++i) {
        grep_code = get_entity_for_entry(entries[i], etty_c_name, extracted_name);
        
        if (grep_code < 0) {
            debug_print("entity could not be fetched from entry %s, reason: %d\n", entries[i], grep_code);

            return grep_code;
        }

        for (b = 0; b < ret_code; ++b) {
            if (strcmp(out_buf[b], extracted_name) == 0) {
                goto pass;
            }
        }

        out_buf[ret_code] = malloc(sizeof(char) * BUFSIZ);
        strcpy(out_buf[ret_code], extracted_name);
        ret_code += 1;
pass:
        /* pass */;
    }

    free(extracted_name);

    return ret_code;
}

int32_t normalize_entries_for_district(const char ** restrict entries, uint32_t len, char ** restrict out_buf) {
    size_t i, b;
    char *extracted_name = malloc(sizeof(char) * BUFSIZ);
    int32_t ret_code = 0UL, grep_code = 0UL;

    debug_print("normalization started for len: %d\n", len);

    for (i = 0; i < len; ++i) {
        grep_code = get_entity_for_entry(entries[i], etty_d_name, extracted_name);

        if (grep_code < 0) {
            debug_print("entity could not be fetched from entry %s, reason %d\n", entries[i], grep_code);

            return grep_code;
        }


        for (b = 0; b < ret_code; ++b) {
            if (strcmp(out_buf[b], extracted_name) == 0) {
                goto pass;
            }
        }

        out_buf[ret_code] = malloc(sizeof(char) * BUFSIZ);
        strcpy(out_buf[ret_code], extracted_name);
        ret_code += 1;
pass:
        /* pass */;
    }

    free(extracted_name);

    return ret_code;
}


int32_t normalize_entries_for_neighbor(const char ** restrict entries, uint32_t len, char ** restrict out_buf) {
    size_t i, b;
    char *extracted_name = malloc(sizeof(char) * BUFSIZ);
    int32_t ret_code = 0UL, grep_code = 0UL;

    for (i = 0; i < len; ++i) {
        grep_code = get_entity_for_entry(entries[i], etty_n_name, extracted_name);

        if (grep_code < 0) {
            debug_print("entity could not be fetched from entry %s, reason %d\n", entries[i], grep_code);

            return grep_code;
        }

        for (b = 0; b < ret_code; ++b) {
            if (strcmp(out_buf[b], extracted_name) == 0) {
                goto pass;
            }
        }

        out_buf[ret_code] = malloc(sizeof(char) * strlen(extracted_name));
        strcpy(out_buf[ret_code], extracted_name);
        ret_code += 1;
pass:
        /* pass */;
    }

    free(extracted_name);

    return ret_code;
}

int32_t dump_entry(const char * restrict entry, char * restrict out_buf) {
    int32_t ret_code = 0UL;
    uint32_t zip = 0UL;
    char *c_name = malloc(sizeof(char) * BUFSIZ);
    char *d_name = malloc(sizeof(char) * BUFSIZ);
    char *n_name = malloc(sizeof(char) * BUFSIZ);
    char *lat = malloc(sizeof(char) * BUFSIZ);
    char *lon = malloc(sizeof(char) * BUFSIZ);

    if (entry == NULL || out_buf == NULL) {
        ret_code = -EINVAL;

        goto bailout;
    }

    if (sscanf(entry, "%u\t%s\t%s\t%s\t%s\t%s", &zip, n_name, c_name, d_name, lat, lon) != 6) {
        ret_code = -EINVAL;

        goto bailout;
    }

    sprintf(out_buf, "code: %u\nneighborhood: %s\ncity: %s\ndistrict: %s\nlatitude: %s\nlongitude: %s\n", zip, n_name, c_name, d_name, lat, lon);

bailout:
    return ret_code;
}
