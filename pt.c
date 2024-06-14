#include "os.h"

typedef struct node
{
    struct node** childs;
} node;

void create_mapping(uint64_t, uint64_t , uint64_t );

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    if(ppn == NO_MAPPING) {
        // delete_mapping(pt, vpn);
        return;
    }
    create_mapping(pt, vpn, ppn);
    return;
}

void create_mapping(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    node *head = pt;
    int mask = 0;
    int chunk = 0;
    while ( vpn != 0){
        chunk = vpn & mask;
        head = create_node(head, chunk);
        vpn = vpn << 9;

    }
}