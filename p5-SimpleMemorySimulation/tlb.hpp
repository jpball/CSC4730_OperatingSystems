#pragma once

#include <vector>
#include <cstdint>
#include "arch.hpp"


// Translation Lookaside Buffer Entry
// via Perry (as PMME)
struct TLB_ENTRY {
	TLB_ENTRY() = default;
	bool in_use;
	uint32_t VPN;
	uint32_t PFN;
};

// Will hold several TLB_ENTRY's
// And allow for querying and such
class TLB
{
public:
    TLB();
    bool SearchVPN(const uint16_t& vpn, TLB_ENTRY& returnEntry);
    void AddEntry(const uint32_t& vpn, const uint32_t& pfn);
    void DumpTLB();

private:
    const size_t MAX_ENTRIES;
    size_t nextEntry;
    std::vector<TLB_ENTRY> tlbEntries;
};
