#pragma once

#include "csvfs.h"

#include <stdint.h>

struct district_t;

struct neighborhood_t {
    struct district_t *dist;
    char *name;
    uint32_t code;
    char *lat;
    char *lon;
};

/**
 * neighborhood_dump_cstr_create
 *
 * Dumps a neighborhood struct to a C-string, which is allocated by the function itself.
 * The function applies to `The Create Rule`, the memory taken for `dmp_str` should be freed
 * in the responsibility of the user.
 *
 * @param ngh The neighborhood to dump.
 * @param dmp_str Pointer to the C-string.
 */
CSVFS_EXPORT int32_t neighborhood_dump_cstr_create(const struct neighborhood_t * restrict ngh, char ** restrict dmp_str);

