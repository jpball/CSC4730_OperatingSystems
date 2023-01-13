#include "FileSystem.hpp"

FileSystem::FileSystem(const std::string PATH)
{
	// Create enough of a buffer on the heap to hold all of the image's binary
	p_fullImage = new char[IMAGE_SIZE];

	// Read in the image binary to the fullImage array
	std::ifstream fin(PATH, std::ifstream::in | std::ifstream::binary);

	if (!fin.is_open())
	{
		std::string message = std::string("Could not open file: " + PATH);
		throw FileSystemException(message);
	}

	fin.read((char *)p_fullImage, IMAGE_SIZE);
	fin.close();

	// Superblock is the accurate form
	p_sblock = (superblock *)((char *)p_fullImage + BSIZE);

	// Initalize our inode lists
	allInodes = GatherAllInodes();
	allAllocatedInodes = GatherAllAllocatedInodes();
	allDirInodes = GatherAllDirInodes();

	for(uint i = 0; i < p_sblock->size; i++)
	{
		allBitmapBlocks.push_back(GetBitmapValue(i));
	}


	DisplaySuperblockInfo();
}
//--
FileSystem::~FileSystem()
{
	delete[] (char *) p_fullImage;
}
//--
/*
=======================
	VALIDATION FUNCTIONS
=======================
*/
//--
bool FileSystem::ValidateNullInode()
{
	if(IsInodeAllocated(GetInode(0)))
	{
		std::string message = "Inode 0 is allocated and should not be.";
		throw FileSystemException(message);
	}
	return false;
}
//--
bool FileSystem::ValidateSuperblock()
{
	if(p_sblock->size != NUMBER_TOTAL_BLOCKS)
	{
		std::string message = "Superblock contains an invalid size {Expected: " + std::to_string(NUMBER_TOTAL_BLOCKS) + " Actual: " + std::to_string(p_sblock->size) + "}";
		throw FileSystemException(message);
	}

	if(p_sblock->inodestart != CalcStartingInodeBlock())
	{
		std::string message = "Superblock contains an invalid inodestart {Expected: " + std::to_string(CalcStartingInodeBlock()) + " Actual: " + std::to_string(p_sblock->inodestart) + "}";
		throw FileSystemException(message);
	}

	if(p_sblock->bmapstart != CalcStartingBitmapBlock())
	{
		std::string message = "Superblock contains an invalid bmapstart {Expected: " + std::to_string(CalcStartingBitmapBlock()) + " Actual: " + std::to_string(p_sblock->bmapstart) + "}";
		throw FileSystemException(message);
	}

	if(CalcSystemSize() != NUMBER_TOTAL_BLOCKS)
	{
		std::string message = "System size does not add up! Expected: " + std::to_string(NUMBER_TOTAL_BLOCKS) + " Actual: " + std::to_string(CalcSystemSize());
		throw FileSystemException(message);
	}

	
	
	return true;
}
//--
/*
Missing block
	The bit map says block n is allocated 
	but you cannot find it belonging to any file or directory.
*/
bool FileSystem::CheckForMissingBlock()
{
	uint missingBlock = 0;
	for(uint blockNum = 0; blockNum < p_sblock->nblocks; blockNum++)
	{
		// For each bit in the bitmap corresponding to a data block...
		uint bitmapOffset = blockNum + CalcDataBlockStart();
		if(GetBitmapValue(bitmapOffset) == 1)
		{
			// It is marked as allocated
			if(GetRefCount(bitmapOffset).size() == 0)
			{
				// But it is not referenced by any Inode
				missingBlock = bitmapOffset;
				break;
			}
		}
	}

	// Now that we have found all missing blocks...
	if(missingBlock != 0)
	{
		std::string message = "Block " + std::to_string(missingBlock) + " is marked as allocated on the bitmap, but is not referenced by any inode.";
		throw FileSystemException(message);
	}	

	return false;
}
//--
/*
Unallocated block
	The bit map says block n is free but you found it referenced by a valid inode.
*/
bool FileSystem::CheckForUnallocatedBlock()
{
	uint unallocatedBlock = 0;
	for(uint blockNum = 0; blockNum < p_sblock->nblocks; blockNum++)
	{
		// For each bit in the bitmap corresponding to a data block...
		uint bitmapOffset = blockNum + CalcDataBlockStart();
		if(GetBitmapValue(bitmapOffset) == 0)
		{
			// It is marked as free
			if(GetRefCount(bitmapOffset).size() > 0)
			{
				// But it is not referenced by any Inode
				unallocatedBlock = bitmapOffset;
				break;
			}
		}
	}

	// Now that we have found all missing blocks...
	if(unallocatedBlock != 0)
	{
		std::string message = "Block " + std::to_string(unallocatedBlock) + " is marked as free on the bitmap, but is referenced by a valid inode.";
		throw FileSystemException(message);
	}

	return false;
}
//--
/*
Multiply allocated block
	An allocated block is referenced by 
	more than one inode, or more than once by the same inode.
*/
bool FileSystem::CheckForMultiplyAllocatedBlock()
{
	uint multiplyAllocatedBlock = 0;
	InodeLocs referencedInodes;
	for(uint blockNum = CalcDataBlockStart(); blockNum < p_sblock->nblocks + CalcDataBlockStart(); blockNum++)
	{
		// For each block
		if(GetRefCount(blockNum).size() > 1)
		{
			multiplyAllocatedBlock = blockNum;
			referencedInodes = GetRefCount(blockNum);
			break;
		}
	}

	if(multiplyAllocatedBlock != 0)
	{
		std::string message = "Block " + std::to_string(multiplyAllocatedBlock) + " is referenced multiple times by the following inodes: ";
		for(size_t index = 0; index < referencedInodes.size(); index++)
		{
			message += std::to_string(referencedInodes[index].inodeNum);
			if(index < referencedInodes.size() - 1)
			{
				message += ", ";
			}
		}
		throw FileSystemException(message);
	}
	return false;
}
//--
/*
Missing inode
	An inode number is found in a valid directory 
	but is not marked as allocated in the inode array.
*/
bool FileSystem::CheckForMissingInode()
{
	uint missingInodeNumber = 0;
	for(InodeLoc dirLoc : allDirInodes)
	{
		// Read in all of the directory entries
		std::vector<dirent*> validEntries = GetAllValidDirectoryEntries(dirLoc.inode);
		for(dirent* entry : validEntries)
		{
			// For each entry in the block
			if(IsInodeAllocated(entry->inum) == false)
			{
				// Inode is not allocated
				missingInodeNumber = entry->inum;
				break;
			}
		}
	}
	
	
	if(missingInodeNumber != 0)
	{
		std::string message = "Inode " + std::to_string(missingInodeNumber) + " is referenced by a directory but is not allocated.";
		throw FileSystemException(message);
	}
	return false;
}
//--
/*
Unused inode
	An inode marked valid is not referred to by a directory.
*/
bool FileSystem::CheckForUnusedInode()
{
	uint unusedInodeNumber = 0;
	for(InodeLoc validInode : allAllocatedInodes)
	{
		// For each valid inode in the inode array...
		if(IsInodeReferenced(validInode.inodeNum) == false)
		{
			// Inode is not referenced by any directory
			unusedInodeNumber = validInode.inodeNum;
			break;
		}
	}

	if(unusedInodeNumber != 0)
	{
		std::string message = "Inode " + std::to_string(unusedInodeNumber) + " is allocated but not referenced by any directory.";
		throw FileSystemException(message);
	}

	return false;
}
//--
bool FileSystem::ValidateRootNode()
{
	dinode* rootNode = GetInode(ROOTINO);
	// Ensure root inode is allocated
	if(IsInodeAllocated(rootNode) == false)
	{
		std::string message = "Root inode is marked as unallocated.";
		throw FileSystemException(message);
	}

	// Ensure that the root inode is a directory
	if(rootNode->type != T_DIR)
	{
		std::string message = "Root inode is not a directory.";
		throw FileSystemException(message);
	}

	// Ensure that the root inode refers to itself as '.' and '..'
	uint firstBlock = rootNode->addrs[0];
	dirent* selfRef = GetDirectoryEntry(firstBlock, 0);
	dirent* parentRef = GetDirectoryEntry(firstBlock, 1);

	if(selfRef->inum != ROOTINO)
	{
		std::string message = "Root directory does not refer to itself via '.'";
		throw FileSystemException(message);
	}

	if(parentRef->inum != ROOTINO)
	{
		std::string message = "Root directory does not refer to itself via '..'";
		throw FileSystemException(message);
	}

	return false;
}
//--
bool FileSystem::CheckForInvalidSpecialEntryNames()
{
	for(InodeLoc dirLoc : allDirInodes)
	{
		// For each directory Inode...
		dirent* selfRef = GetDirectoryEntry(dirLoc.inode->addrs[0], 0);
		dirent* parentRef = GetDirectoryEntry(dirLoc.inode->addrs[0], 1);

		if(strcmp(selfRef->name, ".") != 0)
		{
			std::string message = "Directory " + std::to_string(dirLoc.inodeNum) + "'s first file name is not '.'; Actual: " + selfRef->name;
			throw FileSystemException(message);
		}

		if(strcmp(parentRef->name, "..") != 0)
		{
			std::string message = "Directory " + std::to_string(dirLoc.inodeNum) + "'s first file name is not '..'; Actual: " + parentRef->name;
			throw FileSystemException(message);
		}
	}
	return false;
}
//--
bool FileSystem::CheckForDirectoryLoop()
{
	std::vector<uint> path;

	InodeLoc rootNode = GetInodeLoc(ROOTINO);
	path.push_back(ROOTINO);
	std::vector<dirent*> childEntries = GetAllValidDirectoryEntries(rootNode.inode);
	for(dirent* entry : childEntries)
	{
		if(strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0)
		{
			// We don't care about the '.' and '..' entries
			continue;
		}
		// For each child entry in the root directory...
		// Search it's children for a loop
		InodeLoc childNode = GetInodeLoc(entry->inum);
		if(childNode.inode->type == T_DIR)
		{
			SearchForDirLoop(path, childNode);
		}
	}

	return false;
}
//--
bool FileSystem::CheckForNonDirSpecialEntries()
{
	for(InodeLoc dirLoc : allDirInodes)
	{
		// For each directory inode..
		dirent* selfRef = GetDirectoryEntry(dirLoc.inode->addrs[0], 0);
		dirent* parentRef = GetDirectoryEntry(dirLoc.inode->addrs[0], 1);

		if(GetInode(selfRef->inum)->type != T_DIR)
		{
			std::string message = "Directory Inode " + std::to_string(dirLoc.inodeNum) + " contains a non-directory as '.'";
			throw FileSystemException(message);
		}
		if(GetInode(parentRef->inum)->type != T_DIR)
		{
			std::string message = "Directory Inode " + std::to_string(dirLoc.inodeNum) + " contains a non-directory as '..'";
			throw FileSystemException(message);
		}
	}
	return false;
}
//--
bool FileSystem::ValidateLinkCounts()
{
	for(InodeLoc inodeLoc : allAllocatedInodes)
	{
		// For each inode...
		uint linkCount = GetLinkCount(inodeLoc);
		if( linkCount != inodeLoc.inode->nlink)
		{
			std::string message = "Inode " + std::to_string(inodeLoc.inodeNum) + " contains an invalid link count; Inode Count: " + std::to_string(inodeLoc.inode->nlink) + " Actual Count: " + std::to_string(linkCount);
			throw FileSystemException(message);
		}
	}
	return false;
}
//--
bool FileSystem::CheckForNonRootSelfRefRoot()
{
	for(InodeLoc dirInode : allDirInodes)
	{
		if(dirInode.inodeNum == ROOTINO) continue; // The root node does not follow this rule
		// For each directory...
		dirent* selfRef = GetDirectoryEntry(dirInode.inode->addrs[0], 0);
		if(strcmp(selfRef->name, ".") == 0 && selfRef->inum == ROOTINO)
		{
			// This inode has listed itself as inode 1
			std::string message = "Directory inode " + std::to_string(dirInode.inodeNum) + " listed itself via '.' as Root Inode (1)";
			throw FileSystemException(message);
		}
	}
	return false;
}
//--
bool FileSystem::ValidateFileSizes()
{
	for(InodeLoc iloc :  allAllocatedInodes)
	{
		// For each inode
		if(iloc.inode->size > MAXFILE_SIZE)
		{
			std::string message = "Inode " + std::to_string(iloc.inodeNum) + " exceeds the maximum file size; MAX: " + std::to_string(MAXFILE_SIZE) + " Actual: " + std::to_string(iloc.inode->size);
			throw FileSystemException(message);
		}
		uint calcSize = CalcInodeSize(iloc.inode);
		if(calcSize != iloc.inode->size)
		{
			std::string message = "Inode " + std::to_string(iloc.inodeNum) + " contains an incorrect size. Inode Size: " + std::to_string(iloc.inode->size) + " Actual: " + std::to_string(calcSize);
			throw FileSystemException(message);
		}
	}
	return false;
}
//--
bool FileSystem::ValidatePrintableNames()
{
	for(InodeLoc dirInode : allDirInodes)
	{
		std::vector<dirent*> allEntries = GetAllValidDirectoryEntries(dirInode.inode);
		for(dirent* entry : allEntries)
		{
			if(! IsNamePrintable(std::string(entry->name)))
			{
				std::string message = "Inode " + std::to_string(dirInode.inodeNum) + " contains an entry with an non-printable name";
				throw FileSystemException(message);
			}
		}
	}
	return false;
}
//--
/*
=======================
	HELPER FUNCTIONS
=======================
*/
//--
std::vector<dirent*> FileSystem::GetValidDirEntriesInBlock(uint blockNum)
{
	std::vector<dirent*> validEntries;
	for(uint entryIndex = 0; entryIndex < DIRENT_PER_BLOCK; entryIndex++)
	{
		dirent* entry = GetDirectoryEntry(blockNum, entryIndex);
		if(entry->inum != 0)
		{
			validEntries.push_back(entry);
		}
	}
	return validEntries;
}

//--
uint FileSystem::CalcInodeSize(dinode* inode)
{
	uint sizeSoFar = 0;
	for(uint addrIndex = 0; addrIndex < NDIRECT; addrIndex++)
	{
		if(inode->addrs[addrIndex] != 0)
		{
			sizeSoFar += BSIZE;
		}
	}

	// Check indirect tree
	if(inode->addrs[NDIRECT] != 0)
	{
		std::vector<uint> indirectVals = GetIndirectBlock(inode);
		for(uint val : indirectVals)
		{
			if(inode->type == T_DIR)
			{
				std::vector<dirent*> validEntries = GetValidDirEntriesInBlock(val);
				sizeSoFar += (validEntries.size() * sizeof(dirent));
			}
			else
			{
				sizeSoFar += BSIZE;
			}
		}
	}
	return sizeSoFar;
}
//--
bool FileSystem::IsNamePrintable(std::string name)
{
	for(auto c : name)
	{
		if(!isprint(c))
		{
			return false;
		}
	}
	return true;
}
//--
uint FileSystem::GetLinkCount(InodeLoc inode)
{
	uint count = 0;
	for(InodeLoc dirInode : allDirInodes)
	{
		// Ignore the counting if it's the directory we're wanting to count
		std::vector<dirent*> allEntries =  GetAllValidDirectoryEntries(dirInode.inode);
		for(dirent* entry : allEntries)
		{
			// For each entry in this directory...
			if(inode.inodeNum == entry->inum)
			{
				count++;
			}
		}
	}
	return count;
}
//--
void FileSystem::SearchForDirLoop(std::vector<uint>& path, InodeLoc currNode)
{
	std::vector<uint>::iterator it = find(path.begin(), path.end(), currNode.inodeNum);

	if(it != path.end())
	{
		// We found this node already in the path!
		std::string message = "Inode " + std::to_string(path.back()) + " refers to a non-parent ancestor (" + std::to_string(*it) + ") thus causing a loop.";
		throw FileSystemException(message);
	}
	else
	{
		// We didn't find it
		path.push_back(currNode.inodeNum);
		std::vector<dirent*> childEntries = GetAllValidDirectoryEntries(currNode.inode);
		for(dirent* entry : childEntries)
		{
			if(strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0)
			{
				// We don't care about the '.' and '..' entries
				continue;
			}

			// For each child entry in the root directory...
			// Search it's children for a loop
			InodeLoc childNode = GetInodeLoc(entry->inum);
			if(childNode.inode->type == T_DIR)
			{
				SearchForDirLoop(path, childNode);
				path.pop_back(); // Pop back
			}
		}
	}
}
//--
bool FileSystem::IsInodeReferenced(uint inodeNum)
{
	for(InodeLoc dirLoc : allDirInodes)
	{
		if(SearchDirectoryForInodeNum(dirLoc.inode, inodeNum))
		{
			return true;
		}
	}
	return false;
}
//--
bool FileSystem::SearchDirectoryForInodeNum(dinode* dir_inode, uint inodeNum)
{
	for(uint addrIndex = 0; addrIndex < NDIRECT; addrIndex++)
	{
		// For each block in the address
		if(dir_inode->addrs[addrIndex] != 0)
		{
			// Address block is valid
			// Read in the block's directory entries
			for(size_t i = 0; i < DIRENT_PER_BLOCK; i++)
			{
				dirent* entry = GetDirectoryEntry(dir_inode->addrs[addrIndex], i);
				if(entry->inum == inodeNum)
				{
					// Entry is valid
					return true;
				}
			}
		}
	}

	if(dir_inode->addrs[NDIRECT] != 0)
	{
		// Indirect valid
		std::vector<uint> indirectTreeVals = GetIndirectBlock(dir_inode);
		for(uint val : indirectTreeVals)
		{
			if(val != 0)
			{
				for(size_t i = 0; i < DIRENT_PER_BLOCK; i++)
				{
					dirent* entry = GetDirectoryEntry(val, i);
					if(entry->inum == inodeNum)
					{
						// Entry is valid
						return true;
					}
				}
			}
		}
	}
	return false;
}
//--
std::vector<dirent*> FileSystem::GetAllValidDirectoryEntries(dinode* dir_inode)
{
	std::vector<dirent*> validEntries;
	for(uint addrIndex = 0; addrIndex < NDIRECT; addrIndex++)
	{
		// For each block in the address
		if(dir_inode->addrs[addrIndex] != 0)
		{
			// Address block is valid
			// Read in the block's directory entries
			for(size_t i = 0; i < DIRENT_PER_BLOCK; i++)
			{
				dirent* entry = GetDirectoryEntry(dir_inode->addrs[addrIndex], i);
				if(entry->inum != 0)
				{
					// Entry is valid
					validEntries.push_back(entry);
				}
			}
		}
	}

	// Check the indirect block
	if(dir_inode->addrs[NDIRECT] != 0)
	{
		// Indirect block is valid
		std::vector<uint> indirectBlockVals = GetIndirectBlock(dir_inode);
		for(uint blockNum : indirectBlockVals)
		{
			// For each block in the indirect block
			for(size_t i = 0; i < DIRENT_PER_BLOCK; i++)
			{
				// Read each directory entry
				dirent* entry = GetDirectoryEntry(blockNum, i);
				if(entry->inum != 0)
				{
					// Entry is valid
					validEntries.push_back(entry);
				}
			}
		}
	}
	return validEntries;
}
//--
/*
Given a block number, go through each of the allocated Inodes
And return any and all inodes that contain this block number
*/
InodeLocs FileSystem::GetRefCount(uint blockNum)
{
	InodeLocs inodeRefs;
	for(InodeLoc loc : allAllocatedInodes)
	{
		for(uint i = 0; i < NDIRECT; i++)
		{
			if(loc.inode->addrs[i] == blockNum)
			{
				inodeRefs.push_back(loc);
			}
		}

		if(loc.inode->addrs[NDIRECT] == blockNum)
		{
			inodeRefs.push_back(loc);
		}

		if(loc.inode->addrs[NDIRECT] != 0)
		{
			std::vector<uint> indirectVals = GetIndirectBlock(loc.inode);
			for(uint val : indirectVals)
			{
				if(val == blockNum)
				{
					inodeRefs.push_back(loc);
				}
			}
		}
	}
	return inodeRefs;
}
//--
InodeLocs FileSystem::GatherAllDirInodes()
{
	InodeLocs allDirs;
	for (uint inodeNum = 0; inodeNum < p_sblock->ninodes; inodeNum++)
	{
		if (GetInode(inodeNum)->type == T_DIR)
		{
			InodeLoc loc = GetInodeLoc(inodeNum);
			allDirs.push_back(loc);
		}
	}
	return allDirs;
}
//--
InodeLocs FileSystem::GatherAllAllocatedInodes()
{
	InodeLocs allocInodes;

	for (uint inodeNum = 0; inodeNum < p_sblock->ninodes; inodeNum++)
	{
		if (IsInodeAllocated(inodeNum))
		{
			InodeLoc loc = GetInodeLoc(inodeNum);
			allocInodes.push_back(loc);
		}
	}

	return allocInodes;
}
//--
InodeLocs FileSystem::GatherAllInodes()
{
	InodeLocs allInodes;

	for (uint inodeNum = 0; inodeNum < p_sblock->ninodes; inodeNum++)
	{
		InodeLoc loc = GetInodeLoc(inodeNum);
		allInodes.push_back(loc);
	}

	return allInodes;
}
//--
std::vector<uint> FileSystem::GetIndirectBlock(dinode* inode)
{
	std::vector<uint> indirectBlockVals;
	uint* indirectBlock = (uint*)GetBlock(inode->addrs[NDIRECT]);
	for(uint i = 0; i < NINDIRECT; i++)
	{
		if(indirectBlock[i] != 0)
		{
			indirectBlockVals.push_back(indirectBlock[i]);
		}
	}
	return indirectBlockVals;
}
/*
=======================
	INLINE FUNCTIONS
=======================
*/
//--
inline uint FileSystem::CalcSystemSize()
{
	return CalcNumBitmapBlocks() + p_sblock->nlog + CalcNumInodeBlocks() + 2 + p_sblock->nblocks;
}
//--
inline dirent* FileSystem::GetDirectoryEntry(uint dirBlockNum, uint entryIndex)
{
	return (dirent*)GetBlock(dirBlockNum) + entryIndex;
}
//--
inline uint FileSystem::GetBitmapValue(uint blockNum)
{
	return (((char*)GetBlock(p_sblock->bmapstart))[blockNum / 8] >> (blockNum % 8)) & 1;
}
//--
inline bool FileSystem::IsInodeAllocated(uint inodeNum)
{
	return IsInodeAllocated(GetInode(inodeNum));
}
//--
inline bool FileSystem::IsInodeAllocated(dinode* inode)
{
	return inode->type != 0;
}
//--
inline InodeLoc FileSystem::GetInodeLoc(uint inodeNum)
{
	return {inodeNum, GetInode(inodeNum)};
}
//--
inline dinode* FileSystem::GetInode(uint inodeNum)
{
	return (dinode*)GetBlock(p_sblock->inodestart) + inodeNum;
}
//--
inline void* FileSystem::GetBlock(uint blockNum)
{
	return (char*)p_fullImage + (blockNum * BSIZE);
}
//--
inline uint FileSystem::CalcStartingBitmapBlock()
{
	return 2 + p_sblock->nlog + CalcNumInodeBlocks();
}
//--
inline uint FileSystem::CalcStartingInodeBlock()
{
	return 2 + p_sblock->nlog;
}
//--
inline uint FileSystem::CalcDataBlockStart()
{
	return p_sblock->size - p_sblock->nblocks;
}
//--
inline uint FileSystem::CalcNumInodeBlocks()
{
	return p_sblock->ninodes / IPB + 1;
}
//--
inline uint FileSystem::CalcNumMetaBlocks()
{
	return 2 + p_sblock->nlog + CalcNumBitmapBlocks() + CalcNumInodeBlocks();
}
//--
inline uint FileSystem::CalcNumBitmapBlocks()
{
	return p_sblock->size / BPB + 1;
}
//--
inline void FileSystem::DisplaySuperblockInfo()
{
	printf("Superblock:\n");
	printf("FS size in blocks:     %d\n", p_sblock->size);
	printf("Number of data blocks: %d\n", p_sblock->nblocks);
	printf("Number of inodes:      %d\n", p_sblock->ninodes);
	printf("Number of log blocks:  %d\n", p_sblock->nlog);
	printf("Number of BM blocks:   %d\n", CalcNumBitmapBlocks());
	printf("Log starting block:    %d\n", p_sblock->logstart);
	printf("Inode starting block:  %d\n", p_sblock->inodestart);
	printf("BM starting block:     %d\n", p_sblock->bmapstart);
	printf("Number of meta blocks: %d\n", CalcNumMetaBlocks());
	printf("Data blocks start at:  %d\n", CalcDataBlockStart());
}
//--