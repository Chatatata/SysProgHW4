#pragma once

#include <stdint.h>

#include "neighborhood.h"

struct city_t;

struct district_t {
    struct city_t *city;
    char *name;
    struct neighborhood_t *neigh_head;
};

