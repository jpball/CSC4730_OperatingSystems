#include "next.hpp"


NextFit::NextFit(const uint32_t& sizeOfRam) : FSM(sizeOfRam)
{
    lastIndex = 0;
}
//--
// Using the NextFit algorithm, manipulate FreeList and AllocatedList
// To return MemAddr referring to the newly allowed chunk
// If no chunk can be allocated, return -1
// Remember to sort the lists after you are done
// If we get to the end of the list, we need to start over at the beginning
// Print out whenever lastIndex is changed
MemAddr NextFit::Alloc(const uint32_t& desiredSize)
{
    printf("Next Fit Starting index: %d Start: %d Length: %d\n", lastIndex, freeList.at(lastIndex).start, freeList.at(lastIndex).size);
    MemAddr retAddr = -1;
    size_t lowerBound = lastIndex;
    size_t upperBound = freeList.size();

    for(size_t nodeIndex = lowerBound; nodeIndex < upperBound; )
    {
        if(freeList.at(nodeIndex).size >= desiredSize)
        {
            // Create an alloc'd node on the allocatedList
            // Node(start, size)
            Node allocedNode(freeList.at(nodeIndex).start, desiredSize);
            allocatedList.push_back(allocedNode);

            // Change the freeList Node to refer to the remaining chunk
            uint32_t newSize = freeList.at(nodeIndex).size - desiredSize;
            if(newSize > 0)
            {
                // WE actually have something remaining at our freeList location
                freeList.at(nodeIndex).start += desiredSize;
                freeList.at(nodeIndex).size -= desiredSize;
            }
            else
            {
                // We have nothing in this node after the allocation
                // So delete it from the list
                freeList.erase(freeList.begin() + nodeIndex);
            }
            retAddr = allocedNode.start;
            lastIndex = nodeIndex;
            break;
        }

        nodeIndex++;
        printf("Next Fit index advanced to: %zu\n", nodeIndex);

        if(nodeIndex == freeList.size())
        {
            nodeIndex = 0; // Force an overflow because the for loop does nodeIndex++
            lowerBound = 0;         // Lower our lowerbound to 0
            upperBound = lastIndex; // Move our upper bound to be everything before out last Index
            continue;
        }
    }
    sort(allocatedList.begin(), allocatedList.end(), CompareNodeByAddress);

    if(freeList.size() > 0)
    {
        printf("Next Fit Ending index: %d Start: %d Length: %d\n", lastIndex, freeList.at(lastIndex).start, freeList.at(lastIndex).size);
    }
    else
    {
        printf("Next Fit Ending index: %d Free List is Empty\n", lastIndex);
    }

    if(retAddr != -1)
    {
        printf("Allocated: %u kibibytes starting at: %u\n", desiredSize, retAddr);
    }
    else
    {
        printf("Cannot allocate: %d kibibytes\n", desiredSize);
    }

    return retAddr;
}