#pragma once

#include <stdint.h>

#include "district.h"

typedef struct city_t address_list_t;

struct city_t {
    struct address_list_t *addr_l;
    char *name;
    int16_t code;
    struct district_t *dist_head;
    struct city_t *next;
};


