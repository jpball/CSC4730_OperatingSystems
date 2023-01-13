#include <cstdio>
#include <cstdint>
#include <cassert>

#include "mmu.hpp"

using namespace std;

MMU::MMU()
{
    // Resize the pageTable array to have the number of pages
    pageTable.resize(VRT_PAGES);
}

/* This will allow us to run individual instructions
 * value defaults to -1 (which causes overflow)
 * enum OPCODE {DUMP_MMU, DUMP_PT, READ, WRITE};
 */
void MMU::RunOperation(const OPCODE &opcode, const VRT_Address &v_addr)
{
    switch (opcode)
    {
    case DUMP_MMU:
    {
        DumpMMU();
        break;
    }
    case DUMP_PT:
    {
        DumpPT();
        break;
    }
    case READ:
    {
        Read(v_addr);
        break;
    }
    case WRITE:
    {
        Write(v_addr);
        break;
    }
    default:
    {
        printf("SOMETHING WENT WRONG\n");
        break;
    }
    }
}

/* Prints out the machine architecture
 *
 * Machine Architecture:
 * Page Size (bits):           11
 * Page Size (bytes):          2048
 * VA Size (bits):             16
 * VA Size (bytes):            65536
 * Physical Memory (bytes):    16384
 * Physical Pages:             8
 */
void MMU::PrintArchitecture()
{
    printf("Machine Architecture:\n");
    printf("%-28s%-u\n", "Page Size (bits):", PAGE_BITS);
    printf("%-28s%-u\n", "Page Size (bytes):", PAGE_SIZE);
    printf("%-28s%-u\n", "VA Size (bits):", VA_SIZE_BITS);
    printf("%-28s%-u\n", "VA Size (bytes):", VA_SIZE);
    printf("%-28s%-u\n", "Physical Memory (bytes):", PHYS_SIZE);
    printf("%-28s%-u\n", "Physical Pages:", PHYS_PAGES);
}

/*
 * When you encounter a DUMP_MMU you
 *     must print the contents of your
 *     MMU's TLB matching the following format exactly:
 *
 * NOTE: TLB class has a DumpTLB() method
 * TLB.DumpTLB();
 * EX:
 *     MMU:
 *     [  0] USED VPN:    7
 *     [  1] USED VPN:   23
 *     [  2] USED VPN:   16
 *     [  3] USED VPN:   28
 *     [  4] USED VPN:   22
 *     [  5] USED VPN:   13
 *     [  6] USED VPN:   15
 *     [  7] USED VPN:    9
 */
void MMU::DumpMMU()
{
    printf("MMU:\n");
    tlb.DumpTLB();
}
//--
/*
 * When you encounter a DUMP_PT command,
 *    you must print the contents of your
 *    simulated linear page table, like so:
 * EX:
 * PAGE TABLE:
 *    [  7] CLEAN PRES IN PFN:    0
 *    [  9] CLEAN PRES IN PFN:    7
 *    [ 13] CLEAN PRES IN PFN:    5
 *    [ 15] DIRTY PRES IN PFN:    6
 *    [ 16] CLEAN PRES IN PFN:    2
 *    [ 22] CLEAN PRES IN PFN:    4
 *    [ 23] CLEAN PRES IN PFN:    1
 *    [ 28] CLEAN PRES IN PFN:    3
 */
void MMU::DumpPT()
{
    printf("PAGE TABLE:\n");
    assert(pageTable.size() > 0);
    for (size_t i = 0; i < pageTable.size(); i++)
    {
        PTE entry = pageTable.at(i);
        if (entry.present == 1)
        {
            printf("[%3lu] %5s %4s IN PFN: %4d\n",
                   i,
                   entry.dirty ? "DIRTY" : "CLEAN",
                   entry.present ? "PRES" : "MISS",
                   entry.pfn);
        }
    }
}

// Called when Read X is the opcode
void MMU::Read(const VRT_Address &v_addr)
{
    printf("Read %d\n", v_addr.value);
    TranslateAddress(v_addr, 'R');
}

// Called when Write X is the opcode
void MMU::Write(const VRT_Address &v_addr)
{
    printf("Write %d\n", v_addr.value);
    TranslateAddress(v_addr, 'W');
}

/* Translates a virtual address into a physical address.
 * Handles both if a page is in the TLB and if it needs to be brought out from the swap device.
 * Works for both reads and writes, provided the appropriate flag
 */
void MMU::TranslateAddress(const VRT_Address &v_addr, const char mode)
{
    uint16_t v_a_vpn = v_addr.virtual_address.vpn;
    TLB_ENTRY outEntry;
    if (tlb.SearchVPN(v_addr.virtual_address.vpn, outEntry))
    {
        // TLB Hit
        // VPN: 0 VA: 1 SUCCESSFUL TRANSLATION TO PFN: 0
        // uint32_t physAddr = outEntry.PFN | v_addr.virtual_address.offset;
        if (mode == 'W')
        {
            printf("VPN: %d VA: %d SUCCESSFUL TRANSLATION TO PFN: %d %s\n", v_a_vpn, v_addr.value, outEntry.PFN, pageTable[outEntry.VPN].dirty == 1 ? "REPEAT WRITE" : "NEWLY DIRTY");
            pageTable[outEntry.VPN].dirty = 1;
        }
        else
        {
            printf("VPN: %d VA: %d SUCCESSFUL TRANSLATION TO PFN: %d\n", v_a_vpn, v_addr.value, outEntry.PFN);
        }
    }
    else
    {
        printf("VPN: %d VA: %d PAGE FAULT\n", v_a_vpn, v_addr.value);

        // Determine if we need to swap an existing page out or not
        if (numberOfUsedPages < PHYS_PAGES)
        {
            /* Physical memory is empty right now, so the faulting page will be put
             * into the next sequential physical frame until they are all used
             */
            pageTable[v_a_vpn].present = 1;
            pageTable[v_a_vpn].pfn = numberOfUsedPages;
            printf("VPN: %d VA: %d ASSIGNING TO PFN: %d\n", v_a_vpn, v_addr.value, pageTable[v_a_vpn].pfn);

            if (mode == 'R')
            {
                printf("VPN: %d VA: %d SWAPPING IN TO PFN: %d\n",
                       v_a_vpn, v_addr.value,
                       pageTable[numberOfUsedPages].pfn);
            }
            else
            {
                printf("VPN: %d VA: %d SWAPPING IN TO PFN: %d %s\n",
                       v_a_vpn, v_addr.value,
                       pageTable[numberOfUsedPages].pfn,
                       pageTable[numberOfUsedPages].dirty == 1 ? "REPEAT WRITE" : "NEWLY DIRTY");
                if (pageTable[numberOfUsedPages].dirty == 0)
                {
                    pageTable[numberOfUsedPages].dirty = 1;
                }
            }

            numberOfUsedPages++;
        }
        else
        {
            SwapIn(v_addr);
            printf("VPN: %d VA: %d SWAPPING IN TO PHYSICAL FRAME: %d\n",
                   v_addr.virtual_address.vpn,
                   v_addr.value,
                   pageTable[v_addr.virtual_address.vpn].pfn);
        }

        // Insert the newly translated address into the TLB
        tlb.AddEntry(v_addr.virtual_address.vpn, pageTable[v_a_vpn].pfn);
    }
}

/*
 * Swap the provided address into memory, kicking out another currently used page
 * "Writes" a dirty page back before swapping it out
 */
void MMU::SwapIn(const VRT_Address &vaToSwapIn)
{
    printf("VPN: %lu SELECTED TO EJECT%s\n", nextPageToSwap, pageTable[nextPageToSwap].dirty == 1 ? " DIRTY" : "");
    if (pageTable[nextPageToSwap].dirty == 1)
    {
        printf("VPN: %lu WRITING BACK\n", nextPageToSwap);
        pageTable[nextPageToSwap].dirty = 0;
    }

    pageTable[nextPageToSwap].present = 0;                 // Mark page to swap as no longer present
    pageTable[vaToSwapIn.virtual_address.vpn].present = 1; // Mark the desired page to swap in
    pageTable[vaToSwapIn.virtual_address.vpn].pfn = pageTable[nextPageToSwap].pfn;
    pageTable[nextPageToSwap].pfn = 0; // Sanity check

    printf("VPN: %d VA: %d ASSIGNING TO: %d\n",
           vaToSwapIn.virtual_address.vpn,
           vaToSwapIn.value,
           pageTable[vaToSwapIn.virtual_address.vpn].pfn);

    nextPageToSwap++;

    if (nextPageToSwap >= VRT_PAGES)
    {
        nextPageToSwap = 0;
    }
}
