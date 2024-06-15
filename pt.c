#include <stdio.h>

#include "os.h"

#define CHUNK_SIZE 9
#define OFFSET 12
#define CHUNK_MASK 0xff800000000
#define VPN_SIZE 45


void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    uint64_t *node;
    node = (uint64_t *)phys_to_virt(pt << OFFSET);
    uint64_t chunk;

    for (int i = 0; i < VPN_SIZE/CHUNK_SIZE; i++)
    {
        chunk = (vpn & CHUNK_MASK) >> (VPN_SIZE - CHUNK_SIZE);
        vpn = vpn << CHUNK_SIZE;
        if (node[chunk] == 0)
        {
            node[chunk] = (alloc_page_frame() << OFFSET);
        }
        node = (uint64_t *)phys_to_virt(node[chunk]);

    }
    if(ppn == NO_MAPPING){
        node[chunk] = ppn;
        return;
    }
    // intreduce offset and set valid to 1
    node[chunk] = ((ppn<<OFFSET) | 1);
    return;
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    uint64_t *node;
    node = (uint64_t *)phys_to_virt(pt << OFFSET);
    uint64_t chunk;

    for (int i = 0; i < VPN_SIZE/CHUNK_SIZE; i++)
    {   
        chunk = (vpn & CHUNK_MASK) >> (VPN_SIZE - CHUNK_SIZE);
        vpn = vpn << CHUNK_SIZE;
        if (node[chunk] == 0)
        {
            return NO_MAPPING;
        }
        node = (uint64_t*)phys_to_virt(node[chunk]);
    }
    if (node[chunk] == 0 || node[chunk] == NO_MAPPING)
    {
        return NO_MAPPING;
    }
    return (node[chunk] >> OFFSET);
}