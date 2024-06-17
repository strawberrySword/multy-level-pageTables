#include <stdio.h>

#include "os.h"

#define CHUNK_SIZE 9
#define OFFSET 12
#define CHUNK_MASK 0x1ff000000000
#define INVALID_MASK 0xffffffffffe
#define VPN_SIZE 45

void printBits(size_t const size, void const *const ptr)
{
    unsigned char *b = (unsigned char *)ptr;
    unsigned char byte;
    int i, j;
    for (i = size - 1; i >= 0; i--)
    {
        for (j = 7; j >= 0; j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn)
{
    uint64_t *node;
    node = (uint64_t *)phys_to_virt(pt << OFFSET);
    uint64_t chunk;
    // printf("new insert!!!\n");
    // printBits(sizeof(uint64_t), &vpn);
    for (int i = 0; i < VPN_SIZE / CHUNK_SIZE - 1; i++)
    {
        chunk = (vpn & CHUNK_MASK) >> (VPN_SIZE - CHUNK_SIZE);
        // printf("at chunk %li\n", chunk);
        // printBits(sizeof(uint64_t), &chunk);
        vpn = vpn << CHUNK_SIZE;
        if (node[chunk] == 0)
        {
            node[chunk] = (alloc_page_frame() << OFFSET);
        }
        node = (uint64_t *)phys_to_virt(node[chunk]);
    }
    chunk = (vpn & CHUNK_MASK) >> (VPN_SIZE - CHUNK_SIZE);
    printf("at chunk %li \n", chunk);
    if (ppn == NO_MAPPING)
    {
        node[chunk] = ((node[chunk] >> 1) << 1);
        return;
    }
    // intreduce offset and set valid to 1
    node[chunk] = ((ppn << OFFSET) | 1);
    return;
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn)
{
    uint64_t *node;
    node = (uint64_t *)phys_to_virt(pt << OFFSET);
    uint64_t chunk;

    for (int i = 0; i < VPN_SIZE / CHUNK_SIZE -1; i++)
    {
        chunk = (vpn & CHUNK_MASK) >> (VPN_SIZE - CHUNK_SIZE);
        // printf("at chunk %li \n", chunk);
        vpn = vpn << CHUNK_SIZE;
        if (node[chunk] == 0)
        {
            return NO_MAPPING;
        }
        node = (uint64_t *)phys_to_virt(node[chunk]);
    }
    chunk = (vpn & CHUNK_MASK) >> (VPN_SIZE - CHUNK_SIZE);
    // printBits(sizeof(uint64_t), &node[chunk]);
    // printBits(sizeof(uint64_t), &vpn);
    if ((node[chunk] & 1) == 0)
    {
        printf("invalid\n");
        return NO_MAPPING;
    }
    printf("valid\n");
    return (node[chunk] >> OFFSET);
}