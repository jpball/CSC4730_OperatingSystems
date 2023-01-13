/*
*********************************************
P5 - Make believe Address Translation
CSC 4730 - MWF 14:50 - 15:55

	Written by Jordan Ball & Joey Sachtleben
*********************************************
*/

#include <iostream>
#include <string>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cinttypes>

#include "mmu.hpp"

// Symbolic constants

// Error values to use for retval/exit
const int32_t MISSING_FILE_NAME = 1;
const int32_t FAILED_OPENING_FILE = 11;
const int32_t INVALID_INSTRUCTION_READ = 12;
const int32_t INVALID_ADDRESS_READ = 13;


using namespace std;

struct Instruction { OPCODE opcode; VRT_Address arg; };

bool HandleOptions(int argc, char* argv[], string& filePath, bool& debug);
void ReadAndExecFile(MMU& mmu, const string& FILE_PATH);
Instruction ParseInstruction(const string& line);

void CommandLine(MMU& mmu); // debugging prompt

int main(int argc, char * argv[]) {
	string filePath("");
	bool debug = false;

	// Check that we have a valid number of arguments
	if(argc < 2 || !HandleOptions(argc, argv, filePath, debug))
	{
		fprintf(stderr, "Must specify filename with -f\n");
		exit(MISSING_FILE_NAME);
	}

	// Assert that we have something in the filePath
	// File validity is checked in ReadAndExecFile()
	if (!debug)
	{
		assert(filePath.size() > 0);
		MMU mmu;
		mmu.PrintArchitecture();
		ReadAndExecFile(mmu, filePath);
	}
	else
	{
		MMU mmu;
		CommandLine(mmu);
	}

	return 0;
}


void ReadAndExecFile(MMU& mmu, const string& filePath)
{
	ifstream fin(filePath);
	if(!fin.is_open())
	{
		// File not opened
		fprintf(stderr, "Failed to open: %s\n", filePath.c_str());
		perror("Cause:");
		exit(FAILED_OPENING_FILE);
	}

	// At this point, the file is open in fin and is ready
	string readBuffer;
	while(fin.good())
	{
		getline(fin, readBuffer);
		if(readBuffer.size() > 0)
		{
			Instruction ins = ParseInstruction(readBuffer);
			mmu.RunOperation(ins.opcode, ins.arg);
		}
	}

	fin.close();
}

// enum OPCODE {DUMP_MMU, DUMP_PT, READ, WRITE};
Instruction ParseInstruction(const string& line)
{
	stringstream sin(line);
	Instruction ins;
	string readBuffer;

	// Read in the first part of the line
	// Which is likely the OPCODE
	// Will error if not a valid opcode
	sin >> readBuffer;
	if(readBuffer == "Read")
	{
		ins.opcode = READ;
	}
	else if(readBuffer == "Write")
	{
		ins.opcode = WRITE;
	}
	else if(readBuffer == "DUMP_PT")
	{
		ins.opcode = DUMP_PT;
		return ins;
	}
	else if(readBuffer == "DUMP_MMU")
	{
		ins.opcode = DUMP_MMU;
		return ins;
	}
	else
	{
		fprintf(stderr, "Invalid instruction read: %s\n", readBuffer.c_str());
		exit(INVALID_INSTRUCTION_READ);
	}

	// Read the instruction's argument
	// NOTE: If OPCODE is DUMP_PT or DUMP_MMU, they will not reach this point
	// because they do not require an argument
	// Only READ and WRITE will reach this point
	assert(ins.opcode == READ || ins.opcode == WRITE);

	// String Streams automatically convert string to our desired variable
	// In this case, uint16_t
	sin >> ins.arg.value;
	if (sin.fail())
	{
		fprintf(stderr, "ERROR: Invalid address specified\n");
		exit(INVALID_ADDRESS_READ);
	}
	// If we've gotten to this point, all should be good
	return ins;
}

// Read in the command line options
// -f => filePath (REQUIRED)
bool HandleOptions(int argc, char* argv[], string& filePath, bool& debug)
{
	int c;
	while ((c = getopt(argc, argv, "f:d")) != -1)
	{
		switch(c)
		{
			case 'f':
			{
				filePath = string(optarg);
				break;
			}
			case 'd':
			{
				debug = true;
				break;
			}
		}
	}
	return filePath.size() > 0;
}

void CommandLine(MMU& mmu)
{
	string input;
	cout << "> ";
	getline(cin, input);
	while (input != "quit")
	{
		Instruction i = ParseInstruction(input);
		mmu.RunOperation(i.opcode, i.arg);
		cout << "> ";
		getline(cin, input);
	}

}
