#include <iostream>
#include <string>
#include <unistd.h>
#include <sstream>
#include "best.hpp"

using namespace std;

void PromptInstruction(BestFit& bf);
void ProcessInstruction(BestFit& bf, const char& instr, const int32_t& arg = -1);
void HandleOptions(const int& argc, char* argv[], uint32_t& memSize);
void PrintUsage();

int main(int argc, char * argv[]) {
	uint32_t memSize = 512;
	if(argc > 1)
	{
		HandleOptions(argc, argv, memSize);
	}


	BestFit bf(memSize);

	PromptInstruction(bf);
	return 0;
}
//--
void PromptInstruction(BestFit& bf)
{
	char instr;
	int32_t arg = -1;
	string read, temp;

	if(isatty(STDIN_FILENO))
	{
		printf(">");
	}

	while(getline(cin, read))
	{
		// Parse the string into the 2 components
		// instr & arg
		stringstream SIN(read);
		getline(SIN, temp, ' ');
		instr = temp.c_str()[0];

		if(SIN.good() && instr != '#')
		{
			getline(SIN, temp);
			arg = stoi(temp);
		}

		if(instr == 'q')
		{
			// Use 'q' to break out of the loop
			break;
		}
		else
		{
			ProcessInstruction(bf, instr, arg);
		}

		if(isatty(STDIN_FILENO))
		{
			printf(">");
		}
	} 
	
}
//--
/*
	Command	Argument	Meaning
	q	None	Exits the program
	p	None	Prints the Free and Allocated lists
	a	size	Attempts to Allocate size kibibytes
	f	addr	Attempts to Free an allocation starting at addr
	#	text	A # means skip the entire line
*/
void ProcessInstruction(BestFit& bf, const char& instr, const int32_t& arg)
{
	switch (instr)
	{
	case 'p':
		bf.PrintFreeList();
		bf.PrintAllocList();
		break;
	case 'a':
		bf.Alloc(arg);
		break;
	case 'f':
		bf.Free(arg);
		break;
	default:
		break;
	}
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