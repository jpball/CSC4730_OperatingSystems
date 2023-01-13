#include "slab.hpp"

SlabFit::SlabFit(const uint32_t& memSize)
{
    
}
SlabFit::~SlabFit()
{
    while(chunkStarts.size() > 0)
    {
        printf("Releasing Gulp\n");
        void* p_head = chunkStarts.front(); // Grab the start of the malloc'd chunk
        free(p_head); // Free that malloc'd chunk
        chunkStarts.pop_front(); // Remove the chunk pointer from the queue
    }
}
//--
Slab_ptr SlabFit::AllocSlab()
{
    if(freeList.size() == 0)
    {
        // WE NEED MORE SLABS
        Gulp();
    }
    
    Slab_ptr nextSlabPtr = freeList.top();
    freeList.pop();
    allocatedList.push_front(nextSlabPtr); // Add it to our alloccedList to keep track
    printf("Allocating Slab %4lu\n", freeList.size());
    return nextSlabPtr;
}
//--
void SlabFit::FreeSlab(Slab_ptr ptr)
{
    if(ptr == nullptr)
    {
        throw "attempting to free nullptr";
    }
    if(!IsAddressInGulp(ptr))
    {
        // If we reach this point, then IsAddressInAllocList == false && IsAddressInFreeList == false
        // Thus the ptr is in neither list
        throw "attempting to free location not in any gulp";
    }
    if(freeList.size() == GULP_SIZE * chunkStarts.size())
    {
        // Our freeList is full, so we are trying to free when we have nothing to free
        throw "free'd too many slabs";
    }
    if(IsAddressInFreeList(ptr))
    {
        throw "attempting double free";
    }

    // ptr is within bounds
    freeList.push(ptr);
    allocatedList.remove(ptr);
}
// --
void SlabFit::Gulp()
{
    printf("Gulping\n");
    void* p_head = malloc(GULP_SIZE * SLAB_SIZE); // ALLOCATE A CHUNK OF MEMORY FOR OUR SLABS
    chunkStarts.push_back(p_head); // Add the beginning of this chunk to a queue for free'ing later

    for(size_t slabNum = 0; slabNum < GULP_SIZE; slabNum++)
    {
        // For a certain number of slabs
        // Add slab referring to the address of the next section of memory we malloc'd
        // (slabNum * SLAB_SIZE) will return the offset from the beginning of our malloc'd chunk
        // (char*)p_head allows us to do address arithmetic by casting our void* to char*
        freeList.push((char*)p_head + (slabNum * SLAB_SIZE));
    }
}
//--
uint32_t SlabFit::GetNumberOfFreeSlabs()
{
    return freeList.size();
}
//--
bool SlabFit::IsAddressInFreeList(const void* address)
{
    bool retVal = false;
    std::stack<void*> freeCopy = freeList;
    while(freeCopy.size() > 0)
    {
        if(freeCopy.top() == address)
        {
            retVal = true;
            break;
        }
        freeCopy.pop();
    }
    return retVal;
}
//--
bool SlabFit::IsAddressInAllocList(const void* address)
{
    bool retVal = false;
    for(auto it = allocatedList.begin(); it != allocatedList.end(); it++)
    {
        if(*it == address)
        {
            retVal = true;
            break;
        }
    }
    return retVal;
}
//--
bool SlabFit::IsAddressInGulp(const void* address)
{
    bool retVal = false;
    for(auto it = chunkStarts.begin(); it != chunkStarts.end(); it++)
    {
        char* p_start = (char*)*it;
        // For each chunk's starting address
        if(address >= p_start && address < (p_start + CHUNK_SIZE))
        {
            // We have found that our address is within this chunk
            retVal = true;
            break;
        }
    }
    return retVal;
}