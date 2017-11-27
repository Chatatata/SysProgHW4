#pragma once

#include "csvfs.h"

#include <stdint.h>

CSVFS_EXPORT int32_t parse_path_str(char * restrict pathstr, char ** restrict pathbuf);

