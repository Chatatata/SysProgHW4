#pragma once

#include <stdint.h>

#include "city.h"

typedef struct city_t address_list_t;

CSVFS_EXPORT int32_t address_list_parse_csv(const char * restrict file_name, address_list_t * restrict addr_list);
