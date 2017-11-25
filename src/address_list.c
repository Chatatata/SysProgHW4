#include "address_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

int32_t address_list_parse_csv(const char * restrict file_name, address_list_t * restrict addr_list) {
    FILE *file;
    char *buf;
    uint32_t zip = 0UL;
    char *n_name;
    char *d_name;
    char *c_name;
    char *lat;
    char *lon;
    struct city_t *cur_city;
    struct district_t *cur_district = NULL;
    
    if (file_name == NULL || addr_list == NULL) {
        return EINVAL;
    }

    file = fopen(file_name, "r");

    if (file == NULL) {
        perror(__FILE__);

        return ENOENT;
    }

    //  Allocate space for buffers
    buf = malloc(sizeof(char) * BUFSIZ);
    memset(buf, 0, sizeof(char) * BUFSIZ);
    
    n_name = malloc(sizeof(char) * BUFSIZ);
    memset(n_name, 0, sizeof(char) * BUFSIZ);

    d_name = malloc(sizeof(char) * BUFSIZ);
    memset(d_name, 0, sizeof(char) * BUFSIZ);

    c_name = malloc(sizeof(char) * BUFSIZ);
    memset(c_name, 0, sizeof(char) * BUFSIZ);

    lat = malloc(sizeof(char) * BUFSIZ);
    memset(lat, 0, sizeof(char) * BUFSIZ);

    lon = malloc(sizeof(char) * BUFSIZ);
    memset(lon, 0, sizeof(char) * BUFSIZ);

    cur_city = addr_list;

    while (fgets(buf, BUFSIZ, file) != NULL) {
        if (sscanf(buf, "%u\t%s\t%s\t%s\t%s\t%s\n", &zip, n_name, c_name, d_name, lat, lon) != 6) {
            goto bailout;
        }

        //  Check whether city has been changed
        if (cur_city->name == NULL || strcmp(cur_city->name, c_name) != 0) {
            //  Initialize the next element of linked list
            cur_city->next = malloc(sizeof(struct city_t));
            memset(cur_city->next, 0, sizeof(struct city_t));

            cur_city = cur_city->next;
            cur_district = NULL;

            //  Scaffold a city struct
            cur_city->name = malloc(sizeof(char) * strlen(c_name));
            strcpy(cur_city->name, c_name);

            cur_city->code = zip / 1000;
        }

        //  Check if there is a district from last copy
        if (cur_district == NULL) {
            cur_district = cur_city->dist_head;
        }

        //  Initialize buffers
        memset(n_name, 0, sizeof(char) * BUFSIZ);
        memset(d_name, 0, sizeof(char) * BUFSIZ);
        memset(c_name, 0, sizeof(char) * BUFSIZ);
        memset(lat, 0, sizeof(char) * BUFSIZ);
        memset(lon, 0, sizeof(char) * BUFSIZ);
    }

bailout:
    free(n_name);
    free(d_name);
    free(c_name);
    free(lat);
    free(lon);
    free(buf);
    fclose(file);

    return 0;
}
