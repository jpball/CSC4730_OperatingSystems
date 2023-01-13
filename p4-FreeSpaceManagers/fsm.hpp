#pragma once

#include <stdio.h>
#include <vector>
#include <algorithm>

typedef int32_t MemAddr;

class FSM
{
public:
    FSM(uint32_t sizeOfRam);
    virtual MemAddr Alloc(const uint32_t& desiredSize); // For default, use first fit
    void Free(const MemAddr& addr);
    void PrintFreeList();
    void PrintAllocList();
protected:
    struct Node
    {
        Node(MemAddr st, uint32_t sz) : start(st), size(sz) {}
        MemAddr start;
        uint32_t size;
    };

    const uint32_t TOTAL_RAM_SIZE;
    std::vector<Node> freeList;
    std::vector<Node> allocatedList;
    void HandleCoalescing();
    static bool CompareNodeByAddress(const Node& a, const Node& b);
};