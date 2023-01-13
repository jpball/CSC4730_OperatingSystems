#pragma once

#include <cinttypes>
#include "arch.hpp"
#include "tlb.hpp"
#include <vector>

enum OPCODE {DUMP_MMU, DUMP_PT, READ, WRITE};

class MMU
{
public:
    MMU();
    // This will allow us to run individual instructions
    void RunOperation(const OPCODE& opcode, const VRT_Address& v_addr);
    void PrintArchitecture();
private:
    void DumpMMU();                     // Called when DUMP_MMU is the opcode
    void DumpPT();                      // Called when DUMP_PT is the opcode
    void Read(const VRT_Address& v_addr);  // Called when Read X is the opcode
    void Write(const VRT_Address& v_addr); // Called when Write X is the opcode
    void TranslateAddress(const VRT_Address& v_addr, const char mode); // Handles translating addresses
    void SwapIn(const VRT_Address& vaToSwapIn);


    // Functions for adding/removing from TLB?
    std::vector<PTE> pageTable;
    size_t nextPageToSwap = 0;
    size_t numberOfUsedPages = 0;
    TLB tlb;
};

