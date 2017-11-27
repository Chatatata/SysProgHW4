#pragma once

#include "csvfs.h"

#include <stdint.h>

enum entity_name_t {
    etty_zip,
    etty_c_name,
    etty_d_name,
    etty_n_name,
    etty_lat,
    etty_lon,
};

CSVFS_EXPORT int32_t read_file(const char * restrict file_name, char ** restrict out_buf);
CSVFS_EXPORT int32_t write_file(const char * restrict file_name, char ** restrict buf, uint32_t len);

CSVFS_EXPORT int32_t fgrep_city_by_name(const char * restrict file_name, const char * restrict c_name, char ** restrict out_buf);
CSVFS_EXPORT int32_t fgrep_city_by_code(const char * file_name, const uint8_t code, char ** restrict out_buf);
CSVFS_EXPORT int32_t fgrep_district_by_name(const char * restrict file_name, const char * restrict d_name, char ** restrict out_buf);

CSVFS_EXPORT int32_t grep_city_by_name(const char ** restrict buf, uint32_t len, const char * restrict c_name, char ** restrict out_buf);
CSVFS_EXPORT int32_t grep_city_by_code(const char ** buf, uint32_t len, const uint8_t code, char ** restrict out_buf);
CSVFS_EXPORT int32_t grep_district_by_name(const char ** restrict buf, uint32_t len, const char * restrict d_name, char ** restrict out_buf);
CSVFS_EXPORT int32_t grep_district_by_code(const char ** restrict buf, uint32_t len, const uint32_t code, char ** restrict out_buf);
CSVFS_EXPORT int32_t grep_neighbor_by_name(const char ** restrict buf, uint32_t len, const char * restrict n_name, char ** restrict out_buf);

CSVFS_EXPORT int32_t get_entity_for_entry(const char * restrict entry, enum entity_name_t etty, void * restrict arg);
CSVFS_EXPORT int32_t normalize_entries_for_zip(const char ** restrict entries, uint32_t len, uint32_t * restrict out_buf);
CSVFS_EXPORT int32_t normalize_entries_for_city(const char ** restrict entries, uint32_t len, char ** restrict out_buf);
CSVFS_EXPORT int32_t normalize_entries_for_district(const char ** restrict entries, uint32_t len, char ** restrict out_buf);
CSVFS_EXPORT int32_t normalize_entries_for_neighbor(const char ** restrict entries, uint32_t len, char ** restrict out_but);

CSVFS_EXPORT int32_t dump_entry(const char * restrict entry, char * restrict out_buf);
