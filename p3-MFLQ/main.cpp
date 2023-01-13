/*
	P3 - CSC 4730 Project 3 - MLFQ
		NO PARTNER USED
*/

#include "MLFQSch.hpp"

using namespace std;

int main(int argc, char * argv[]) {
	MLFQSch sch;
	if(argc > 1){
		// Filename included
		string filePath = string(argv[1]);
		sch.RunInstructionFile(filePath);
	}
	else
	{
		fprintf(stderr, "Missing input file name.\n");
		exit(1);
	}
	return 0;
}
