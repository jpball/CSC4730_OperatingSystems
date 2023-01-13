#pragma once

#include <stdio.h>
#include <string>
#include <getopt.h>
#include <fcntl.h>

using namespace std;

class CommandOptions{
    public:
        CommandOptions(int c, char** v);
        ~CommandOptions();
        void DEBUG_PrintOptionValues();
        
        
        bool IsDEBUG() const { return DEBUG_MODE; }
        string* GetPNewDirPath() const  { return dirPath; }
        string* GetPInPath() const { return inPath; }
        bool IsOverwriting() const { return isOverwrite; }
        string* GetPOutPath() const { return outPath; }
        string* GetPFirstExecPath() const { return firstExecPath; }
        string* GetPSecExecPath() const { return secExecPath; }

    private:
        void HandleOptions();
        void HandleDefaultOptions();
        void PrintUsage();
        
        // Data Members
        int argc;
        char** argv;
        bool DEBUG_MODE;
        string* dirPath;


        string* inPath;
        bool isOverwrite;
        string* outPath;

        string* firstExecPath;
        string* secExecPath;

};