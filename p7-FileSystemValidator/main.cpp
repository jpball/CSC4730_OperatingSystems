#include <stdio.h>
#include <getopt.h>
#include <string>
#include "FileSystem.hpp"

using namespace std;

bool HandleOptions(int argc, char** argv, std::string& path);
void PrintUsage();

int main(int argc, char * argv[]) {

	// Given a binary file path, read it and print it out
	std::string path("");

	if (!HandleOptions(argc, argv, path)) {
		PrintUsage();
		return 1;
	}

	try
	{
		FileSystem fs(path);
		if(fs.ValidateSuperblock())
			printf("Superblock OK\n");

		fs.ValidateNullInode();
		fs.CheckForMissingBlock();
		fs.CheckForUnallocatedBlock();
		fs.CheckForMultiplyAllocatedBlock();
		fs.CheckForMissingInode();
		fs.CheckForUnusedInode();
		fs.ValidateRootNode();
		fs.CheckForInvalidSpecialEntryNames();
		fs.CheckForDirectoryLoop();
		fs.CheckForNonDirSpecialEntries();
		fs.CheckForNonRootSelfRefRoot();
		fs.ValidateLinkCounts();
		fs.ValidateFileSizes();
		fs.ValidatePrintableNames();


	}
	catch (FileSystemException fse)
	{
		printf("%s\n", fse.what());
		exit(55);
	}

	return 0;
}

bool HandleOptions(int argc, char** argv, std::string& path)
{
	char c;
	while ((c = getopt(argc, argv, "hvf:")) != -1)
	{
		switch(c)
		{
			default:
			case('h'):
			{
				return false;
			}
			case('f'):
			{
				path = string(optarg);
				break;
			}
		}
	}
	return path.size() != 0;
}

void PrintUsage()
{
	printf("Usage: [options]\n");
	printf("-f image name\n");
	printf("-h prints this\n");
}


