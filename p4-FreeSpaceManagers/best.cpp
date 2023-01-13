#include "best.hpp"

BestFit::BestFit(const uint32_t& memSize) : FSM(memSize)
{

}
//--
MemAddr BestFit::Alloc(const uint32_t& desiredSize)
{
    MemAddr retAddr = -1;
    uint32_t bestSize = UINT32_MAX;
    size_t bestIndex = 0;
    for(size_t index = 0; index < freeList.size(); index++)
    {
        // For each free block in our freeList
        if(freeList.at(index).size >= desiredSize)
        {
            // This chunk is able to fit our desiredSize
            if(freeList.at(index).size < bestSize)
            {
                // This is the best fit so far!
                bestSize = freeList.at(index).size;
                bestIndex = index;
            }
        }
    }

    if(bestSize != UINT32_MAX)
    {
        // We found something!
        retAddr = freeList.at(bestIndex).start;

        // Create a node on the alloc'd list
        // Node (start, size)
        allocatedList.push_back(Node(retAddr, desiredSize));

        // Adjust our freeList node to be desiredSize smaller
        uint32_t newSize = freeList.at(bestIndex).size - desiredSize;
        if(newSize > 0)
        {
            freeList.at(bestIndex).start += desiredSize;
            freeList.at(bestIndex).size = newSize;
        }
        else
        {
            freeList.erase(freeList.begin() + bestIndex);
        }
    }

    sort(allocatedList.begin(), allocatedList.end(), CompareNodeByAddress);
    
    if(retAddr != -1)
    {
        printf("Allocated: %u kibibytes starting at: %d\n", desiredSize, retAddr);
    }
    else
    {
        printf("Cannot allocate: %u kibibytes\n", desiredSize);
    }
    return retAddr;
}