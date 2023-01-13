#include "fsm.hpp"

FSM::FSM(uint32_t sizeOfRam) : TOTAL_RAM_SIZE(sizeOfRam)
{
    freeList.push_back(Node(0, sizeOfRam)); // Creates the first 
}
//--
void FSM::Free(const MemAddr& addr)
{
    bool foundAddr = false;
    for(size_t nodeIndex = 0; nodeIndex < allocatedList.size(); nodeIndex++)
    {
        // For each Node in the allocatedList
        if(addr == allocatedList.at(nodeIndex).start)
        {
            // The Address 'addr' is referring to the node we are looking at
            foundAddr = true;
            freeList.push_back(allocatedList.at(nodeIndex)); // Adds our node back into the free list
            allocatedList.erase(allocatedList.begin() + nodeIndex); // Erases it from allocated list
            break;
        }
    }
    if(foundAddr)
    {
        sort(freeList.begin(), freeList.end(), CompareNodeByAddress);
        HandleCoalescing();
        printf("Freed block starting at: %d\n", addr);
    }
    else
    {
        printf("Cannot free a block starting at: %d", addr);
    }

}
//--
MemAddr FSM::Alloc(const uint32_t& desiredSize)
{
    MemAddr retAddr = -1;
    for(size_t nodeIndex = 0; nodeIndex < freeList.size(); nodeIndex++)
    {
        if(freeList.at(nodeIndex).size >= desiredSize)
        {
            // Create a node referring to the end of the newly alloc'd node
            MemAddr startAddr = freeList.at(nodeIndex).start + desiredSize;
            freeList.push_back(Node(startAddr, freeList.at(nodeIndex).size - desiredSize));
            
            // Take the old node and use it as our alloc'd node
            Node oldNode = freeList.at(nodeIndex);
            oldNode.size = desiredSize;
            allocatedList.push_back(oldNode);
            freeList.erase(freeList.begin() + nodeIndex);
            retAddr = oldNode.start;
            break;
        }
    }
    sort(allocatedList.begin(), allocatedList.end(), CompareNodeByAddress);
    return retAddr;
}
//--
void FSM::PrintFreeList()
{

    if(freeList.size() == 0)
    {
        printf("Free List is empty\n");
    }
    else
    {
        printf("Free List\n");
        printf(" Index   Start  Length\n");
        for(size_t i = 0; i < freeList.size(); i++)
        {
            printf("[%04zu]%8d%8d\n", i, freeList.at(i).start, freeList.at(i).size);
        }
        printf("\n");
    }
}
//--
void FSM::PrintAllocList()
{
    if(allocatedList.size() == 0)
    {
        printf("Allocated List is empty\n\n");
    }
    else
    {
        printf("Allocated List\n");
        printf(" Index   Start  Length\n");
        for(size_t i = 0; i < allocatedList.size(); i++)
        {
            printf("[%04zu]%8d%8d\n", i, allocatedList.at(i).start, allocatedList.at(i).size);
        }
        printf("\n");
    }
}
//--
void FSM::HandleCoalescing()
{
    // Go through each node in our freeList
    // If the node to the right of it is also in the freeList
    // Expand the current node to consume that memory
    // And delete the rightward one
    for(size_t nodeIndex = 0; nodeIndex < freeList.size() - 1; nodeIndex++)
    {
        if(freeList.at(nodeIndex).start + freeList.at(nodeIndex).size == freeList.at(nodeIndex+1).start)
        {
            // We need to consume the node to the right
            printf("Coalesce at: %d adding %d\n", freeList.at(nodeIndex).start, freeList.at(nodeIndex+1).size); 
            freeList.at(nodeIndex).size += freeList.at(nodeIndex+1).size;
            freeList.erase(freeList.begin() + nodeIndex+1);
            nodeIndex--; // Account for the removal of an item, so we need to move our index back  =
        }
    }
}
//--
bool FSM::CompareNodeByAddress(const Node& a, const Node& b)
{
    return a.start < b.start;
}