#include <stdio.h>
#include "Scheduler.hpp"

using namespace std;

int main(int argc, char * argv[]) {
	Scheduler sch;
	if(argc > 1){
		// Filename included
		string filePath = string(argv[1]);
		sch.RunInstructionFile(filePath);
	}
	return 0;
}
