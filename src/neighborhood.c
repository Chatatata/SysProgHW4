#include "neighborhood.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "district.h"
#include "city.h"

int32_t neighborhood_dump_cstr_create(const struct neighborhood_t * restrict ngh, char ** __restrict dmp_str)
{
    char *tmp;
    int32_t ret_val;

    if (dmp_str == NULL || ngh == NULL) {
        return EINVAL;
    }

    tmp = malloc(sizeof(char) * BUFSIZ);
    memset(tmp, 0, sizeof(char) * BUFSIZ);

    if (tmp == NULL) {
        return ENOMEM;
    }

    ret_val = snprintf(tmp, BUFSIZ, "code: %u\nneighborhood: %s\ncity: %s\ndistrict: %s\nlatitude: %s\nlongitude: %s", ngh->code, ngh->name, ngh->dist->city->name, ngh->dist->name, ngh->lat, ngh->lon);

    if (ret_val < 0) {
        return ret_val;
    } else if (ret_val >= BUFSIZ) {
        return ENOMEM;
    }

    *dmp_str = malloc(sizeof(char) * ret_val);
    memset(*dmp_str, 0, sizeof(char) * ret_val);

    strcpy(*dmp_str, tmp);

    free(tmp);

    return 0;
}

