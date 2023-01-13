#pragma once

#include <stdio.h>
#include <stack>
#include <list>

typedef void* Slab_ptr;

class SlabFit
{
    public:
        SlabFit(const uint32_t& memSize);
        ~SlabFit();
        Slab_ptr AllocSlab();
        void FreeSlab(Slab_ptr ptr);
        uint32_t GetNumberOfFreeSlabs();
    private:   
    const uint16_t GULP_SIZE = 128;    // Each gulp will make 128 slabs
    const uint16_t SLAB_SIZE = 512;    // Each slab will refer to 512 bytes
    const uint32_t CHUNK_SIZE = GULP_SIZE * SLAB_SIZE;
    std::stack<Slab_ptr> freeList;      // Stores the addresses each slab begins at
    std::list<void*> chunkStarts; // Used for deallocating later on
    std::list<void*> allocatedList;

    void Gulp();
    bool IsAddressInFreeList(const void* address);
    bool IsAddressInAllocList(const void* address);
    bool IsAddressInGulp(const void* address);
};