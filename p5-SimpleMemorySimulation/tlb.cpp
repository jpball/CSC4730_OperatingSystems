#include <cstdint>
#include <cstdio>
#include <cassert>

#include "tlb.hpp"

TLB::TLB() : MAX_ENTRIES(PHYS_PAGES), nextEntry(0), tlbEntries(MAX_ENTRIES)
{
}
//--
bool TLB::SearchVPN(const uint16_t& vpn, TLB_ENTRY& returnEntry)
{
    bool retVal = false;
    for(auto tlb_e : tlbEntries)
    {
        if(tlb_e.in_use)
        {
            if(tlb_e.VPN == vpn)
            {
                // Found the TLB_ENTRY with the matching VPN
                retVal = true;
                returnEntry = tlb_e;
                break;
            }
        }
    }
    return retVal;
}

void TLB::AddEntry(const uint32_t& vpn, const uint32_t& pfn)
{
    TLB_ENTRY newEntry;
    newEntry.in_use = true;
    newEntry.VPN = vpn;
    newEntry.PFN = pfn;
    tlbEntries[nextEntry] = newEntry;
    // Check if nextEntry needs to wrap around
    nextEntry = (nextEntry == MAX_ENTRIES - 1 ? 0 : nextEntry+1);
}

void TLB::DumpTLB()
{
    assert(tlbEntries.size() > 0);
    for(size_t i = 0; i < tlbEntries.size(); i++)
    {
        // For each tlbEntry in tlbEntries <vector>
        TLB_ENTRY entry = tlbEntries.at(i);
        // Example -> [  2] USED VPN:   16
        printf("[%3lu] %s VPN:%4d\n",
            i,
            entry.in_use ? "USED" : "FREE",
            entry.VPN
            );
    }
}
