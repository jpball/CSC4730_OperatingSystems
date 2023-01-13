#include "CommandOptions.hpp"

using namespace std;

CommandOptions::CommandOptions(int c, char** v)
{
    this->argc = c;
    this->argv = v;
	DEBUG_MODE = false;
    inPath = nullptr;
    outPath = nullptr;
	isOverwrite = false;
	firstExecPath = nullptr;
    secExecPath = nullptr;

	char buf[PATH_MAX];
	getcwd(buf, PATH_MAX);
	dirPath = new string(buf);

	HandleOptions();
}
//--
CommandOptions::~CommandOptions()
{
	if(dirPath != nullptr){
		delete dirPath;
		dirPath = nullptr;
	}
	if(inPath != nullptr){
		delete inPath;
	}
	if(outPath != nullptr){
		delete outPath;
	}
	if(firstExecPath != nullptr){
		delete firstExecPath;
	}
	if(secExecPath != nullptr){
		delete secExecPath;
	}
}
//--
void CommandOptions::HandleOptions()
{
    int c; // used to receive the arg options
	// NOTE: when getopt is done parsing argv, it returns -1
	while ((c = getopt(argc, argv, "d:i:o:a:1:2:pv")) != -1)
	{
		switch (c)
		{
			case 'd': 
			{
				dirPath = new string(optarg); // Store new working dir. path
				break;
			}

			case 'i':
			{
				inPath = new string(optarg);
				break;
			}

			case 'o':
			{
				outPath = new string(optarg);
				isOverwrite = true;
				break;
			}
			case 'a':
			{
				outPath = new string(optarg);
				isOverwrite = false;
				break;
			}

			case '1':
			{
				firstExecPath = new string(optarg);
				break;
			}

			case '2':
			{
				secExecPath = new string(optarg);
				break;
			}

			case 'p':
			{
				char buf[PATH_MAX];
				printf("== CWD: %s\n", getcwd(buf, PATH_MAX));
			}

			case 'v':
			{
				DEBUG_MODE = true;
				break;
			}

			default:
			{
				printf("Incorrect usage attempted...\n");
				PrintUsage();
				exit(11);
			}
		}
	}
	HandleDefaultOptions();
}
//--
void CommandOptions::HandleDefaultOptions(){
	if(dirPath == nullptr){
		char buf[PATH_MAX];
		getcwd(buf, PATH_MAX);
		dirPath = new string(buf);
	}
	if(firstExecPath == nullptr){
		fprintf(stderr, "Missing required command line option -1\n");
		exit(33);
	}
}
//--
/*
	Print out the command line arguments
		and their proper usages
*/
void CommandOptions::PrintUsage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "-d string (OPT)		path to directory containing commands\n");
    fprintf(stderr, "-i string (OPT)		path to stdin of first program\n");
    fprintf(stderr, "-o string (OPT)		path to stdout of last program (OVERWRITE MODE)\n");
    fprintf(stderr, "-a string (OPT)		path to stdout of last program (APPEND MODE)\n");
    fprintf(stderr, "+++ NOTE: If both -o & -a are given, the latter takes precedence.\n");
    fprintf(stderr, "-1 string (**REQ**)	path to first program to run\n");
    fprintf(stderr, "-2 string (OPT)		path to second program to run / receives input from the first program\n");
    fprintf(stderr, "-p 		(OPT)		prints current working directory\n");
    fprintf(stderr, "-v		(OPT)		prints additional debugging information\n");
}
//--
/*
	This debug function will print out each of the values supplied
*/
void CommandOptions::DEBUG_PrintOptionValues()
{
    printf("==== Option Values DEBUG ========\n");
	printf("   DirectoryPath:  %s\n", 		(dirPath != nullptr) ? dirPath->c_str() : "null");
	printf("   InputFilePath:  %s\n", 		(inPath != nullptr) ? inPath->c_str() : "null");
	printf("  OutputFilePath:  %s (%s)\n", 	(outPath != nullptr) ? outPath->c_str() : "null", isOverwrite ? "OVERWRITE" : "APPEND");
	printf("   FirstProgPath:  %s\n", 		(firstExecPath != nullptr) ? firstExecPath->c_str() : "null");
	printf("  SecondProgPath:  %s\n", 		(secExecPath != nullptr) ? secExecPath->c_str() : "null");
	printf("      DEBUG MODE:  %s\n", 		DEBUG_MODE ? "ON" : "OFF");
	printf("=================================\n");
}
