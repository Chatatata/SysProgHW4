#include <stdio.h>

#include "address_list.h"

int main() {
    address_list_t addr_list;

    address_list_parse_csv("./file.csv", &addr_list);

    return 0;
}
