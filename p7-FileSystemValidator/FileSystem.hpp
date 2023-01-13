#pragma once

#include "fs.h"
#include "types.h"
#include <string>
#include <fstream>
#include <vector>
#include <exception>
#include <deque>
#include <algorithm>


struct InodeLoc
{
    uint inodeNum;
    dinode* inode;
};

typedef std::vector<InodeLoc> InodeLocs;

class FileSystem
{
    public:
    FileSystem(const std::string PATH);
    ~FileSystem();

    // +++++ Validation Functions
    bool ValidateSuperblock();
    bool ValidateNullInode();
    bool CheckForNonRootSelfRefRoot();
    bool CheckForMissingBlock();
    bool CheckForUnallocatedBlock();
    bool CheckForMultiplyAllocatedBlock();
    bool CheckForMissingInode();
    bool CheckForUnusedInode();
    bool ValidateRootNode();
    bool CheckForInvalidSpecialEntryNames();
    bool CheckForDirectoryLoop();
    bool CheckForNonDirSpecialEntries();
    bool ValidateLinkCounts();
    bool ValidateFileSizes();
    bool ValidatePrintableNames();


    private:
    superblock* p_sblock;

    void* p_fullImage;
    
    InodeLocs allDirInodes;
    InodeLocs allInodes;
    InodeLocs allAllocatedInodes;
    std::vector<uint> allBitmapBlocks;


    // +++++ Helper Functions
    bool IsNamePrintable(std::string name);
    void SearchForDirLoop(std::vector<uint>& path, InodeLoc currNode);
    uint GetLinkCount(InodeLoc inode);
    InodeLocs GatherAllInodes();
    InodeLocs GatherAllAllocatedInodes();
    InodeLocs GatherAllDirInodes();
    InodeLocs GetRefCount(uint blockNum);
    bool IsInodeReferenced(uint inodeNum);
    bool SearchDirectoryForInodeNum(dinode* dir_inode, uint inodeNum);
    std::vector<uint> GetIndirectBlock(dinode* inode);
    std::vector<dirent*> GetAllValidDirectoryEntries(dinode* dir_inode);
    uint CalcInodeSize(dinode* inode);
    std::vector<dirent*> GetValidDirEntriesInBlock(uint blockNum);

    // +++++ Inline Functions
    inline dirent* GetDirectoryEntry(uint dirBlockNum, uint entryIndex);
    inline uint GetBitmapValue(uint blockNum);
    inline dinode* GetInode(uint inodeNum);
    inline void* GetBlock(uint blockNum);
    inline bool IsInodeAllocated(uint inodeNum);
    inline bool IsInodeAllocated(dinode* inode);
    inline InodeLoc GetInodeLoc(uint inodeNum);
    inline uint CalcStartingBitmapBlock();
    inline void DisplaySuperblockInfo();
    inline uint CalcDataBlockStart();
    inline uint CalcStartingInodeBlock();
    inline uint CalcNumMetaBlocks();
    inline uint CalcNumInodeBlocks();
    inline uint CalcNumBitmapBlocks();
    inline uint CalcSystemSize();
};

struct FileSystemException : public std::exception
{
    std::string msg;
    FileSystemException(std::string emsg)
    {
        msg = emsg;
    }

    const char * what () const throw () {
      return msg.c_str();
   }
};