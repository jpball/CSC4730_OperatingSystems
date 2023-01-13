#include <iostream>
#include <vector>
#include <string>
#include <getopt.h>
#include "slab.hpp"

using namespace std;

void RunTests(const uint32_t& memSize);
void HandleOptions(const int& argc, char* argv[], uint32_t& memSize);
void PrintUsage();

int main(int argc, char * argv[]) {
	uint32_t memSize = 512;
	if(argc > 1)
	{
		HandleOptions(argc, argv, memSize);
	}
	RunTests(memSize);
	return 0;
}
//--
void RunTests(const uint32_t& memSize)
{
	SlabFit slabManager(memSize);
	bool correctFlag;
	bool errorCaught;
	/* 
	STEP 1
	Allocate 256 slabs consecutively.
		This will cause two gulps and 256 print outs from your allocator.
	You will test to ensure the number of available slabs is exactly 
		0 because step 1 allocates the exact right number of slabs to empty all available.	
	*/
	vector<void*> allSlabs;
	for(size_t i = 0; i < 256; i++)
	{
		allSlabs.push_back(slabManager.AllocSlab());
	}
	correctFlag = slabManager.GetNumberOfFreeSlabs() == 0;
	printf("Number of Available Slabs should be 0. Is: %u (%s)\n", slabManager.GetNumberOfFreeSlabs(), correctFlag ? "Correct" : "Wrong");
	if(!correctFlag)
	{
		// Failed
		exit(1);
	}
	/*
	STEP 2
	All 256 slabs are then freed. 
	The order in which they are freed is not important.

	You will test to ensure that the number of available
		slabs is exactly 256. If the number of available slabs 
		is not 256 you will print the error given to you below 
		and exit the program with return code 1.	
	*/
	printf("Releasing all Allocated Buffers\n");
	for(size_t i = 0; i < allSlabs.size(); i++)
	{
		try
		{
			slabManager.FreeSlab(allSlabs.at(i));
		}
		catch(const char* messages)
		{

			printf("SHOULD NOT CATCH: %s (%zu)\n", messages, i);
			exit(1);
		}
	}
	correctFlag = slabManager.GetNumberOfFreeSlabs() == 256;
	printf("Number of Available Slabs should be 256. Is: %u (%s)\n", slabManager.GetNumberOfFreeSlabs(), correctFlag ? "Correct" : "Wrong");
	if(!correctFlag)
	{
		exit(1);
	}

	/*
	STEP 3
		One slab will be allocated and then freed. Remember its address.
	*/
	Slab_ptr slabA = slabManager.AllocSlab();
	try
	{
		slabManager.FreeSlab(slabA);
	}
	catch(const char* messages)
	{

		printf("SHOULD NOT CATCH: %s\n", messages);
		exit(1);
	}

	/*
	STEP 4
		Another slab will be allocated and then freed. Remember its address.
	*/
	Slab_ptr slabB = slabManager.AllocSlab();
	try
	{
		slabManager.FreeSlab(slabB);
	}
	catch(const char* messages)
	{

		printf("SHOULD NOT CATCH: %s\n", messages);
		exit(1);
	}

	/*
	STEP 5
		The two addresses are compared - they should be the same and reported so.
		If the test fails, your program must exit with a return code of 1.
	*/
	correctFlag = (slabA == slabB);
	printf("Alloc / Free / Alloc Test %s\n", correctFlag ? "succeeded" : "failed");
	if(!correctFlag)
	{
		// Failed
		exit(1);
	}

	/*
	STEP 6
	You will attempt to free a slab giving a null address. 
	This should be flagged as an error. 
	If you don't catch the error, the message given below must be 
		printed and your program exit with return code 1.
	*/
	errorCaught = false;
	try
	{
		slabManager.FreeSlab(nullptr);
	}
	catch(const char* message)
	{
		if(strcmp(message, "attempting to free nullptr") == 0)
		{
			printf("Correctly caught error: %s\n", message);
			errorCaught = true;
		}
	}
	if(!errorCaught)
	{
		printf("Did Not Catch Attempt to Free NULL\n");
		exit(1);
	}
	/*
	STEP 7
	You will attempt to free too many buffers. 
	At this time, all buffers should have been returned to the free pool. 
	Repeat an attempt to free the buffer you remembered in Step 4. 
	Because the number of available buffers reads full, the "too many frees" error should be triggered.
	NOTE THAT YOU SHOULD CHECK FOR TOO MANY FREES BEFORE CHECKING FOR A DOUBLE FREE.

	Step 4's slab is stored in slabB
	*/
	errorCaught = false;
	try
	{
		slabManager.FreeSlab(slabB);
	}
	catch(const char* message)
	{
		if(strcmp(message, "free'd too many slabs") == 0)
		{
			printf("Correctly caught error: %s\n", message);
			errorCaught = true;
		}
	}
	if(!errorCaught)
	{
		printf("Did Not Catch Freeing of Too Many Buffers\n");
		exit(1);
	}

	/*
	STEP 8
	Now allocate TWO slabs. 
	Return the second one TWICE. 
	This should trigger a double free error. 
	If the error is not found, report the failure 
		to catch the error using the text given below and 
		exit your program with a return code of 1.
	*/
	errorCaught = false;
	Slab_ptr slabX = slabManager.AllocSlab();
	Slab_ptr slabY = slabManager.AllocSlab();
	try
	{
		slabManager.FreeSlab(slabY);
		slabManager.FreeSlab(slabY);
	}
	catch(const char* message)
	{
		if(strcmp(message, "attempting double free") == 0)
		{
			printf("Correctly caught error: %s\n", message);
			errorCaught = true;
		}
	}
	if(!errorCaught)
	{
		printf("Did Not Catch Attempt to Double Free\n");
		exit(1);
	}

	try
	{
		slabManager.FreeSlab(slabX); // Free slabX
	}
	catch(const char* messages)
	{

		printf("SHOULD NOT CATCH: %s\n", messages);
		exit(1);
	}

	/*
	STEP 9
	You will attempt to free a make believe slab whose 
		address is 12345. 
	This will, of course, be flagged as an error. 
	If you do not catch the error, you must note this using 
		the text given below. 
	Then your program must exit with a return code of 1.
	*/
	errorCaught = false;
	try
	{
		Slab_ptr ptr = (Slab_ptr)0x12345;
		slabManager.FreeSlab(ptr);
	}
	catch(const char* message)
	{
		if(strcmp(message, "attempting to free location not in any gulp") == 0)
		{
			printf("Correctly caught error: %s\n", message);
			errorCaught = true;
		}
	}
	if(!errorCaught)
	{
		printf("Did Not Catch Attempt to Free BAD Address\n");
		exit(1);
	}

	printf("Program Ending - Destructors should now run\n");
}
//--
void PrintUsage()
{
	printf("Option	Argument				Meaning	               					Default\n");
	printf("-h      none        			prints this help text  					none\n");
	printf("-k      number of Kibibytes		sets the maximal size of free memory	512\n");
}
//--
void HandleOptions(const int& argc, char* argv[], uint32_t& memSize)
{
	int16_t c;
	while((c = getopt(argc, argv, "hk:")) != -1)
	{
		switch(c)
		{
			default:
			case 'h':
			{
				PrintUsage();
				exit(5);
			}
			case 'k':
			{
				memSize = atoi(optarg);
				break;
			}
		}
	}
}