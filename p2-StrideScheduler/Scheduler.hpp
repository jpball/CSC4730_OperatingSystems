#pragma once
#include <stdio.h>
#include <string>
#include <map>
#include <fstream>
#include <list>

#define STRIDE_PROP 10000

enum OPCODE {INVALID, NEWJOB, FINISH, INTERRUPT, BLOCK, UNBLOCK, RUNNABLE, RUNNING, BLOCKED};

class Scheduler{
public:
    Scheduler();
    ~Scheduler();
    void RunInstructionFile(const std::string filePath);
    void RunInstructionString(std::string line);
    void RunCommand(const std::string& com, const std::string& arg1, const int& arg2);
private:

    struct Job{
        Job(std::string n, int p) { name = n; priority = p; pass = 0; stride = STRIDE_PROP / priority; }

        std::string name;
        uint32_t stride;
        uint32_t pass;
        int priority;
    };

    // Methods
    Job* GrabMinPassJob();
    OPCODE ParseCode(const std::string code);
    void CreateNewJob(const std::string name = "", const int &priority = -1);
    void Reschedule();
    static bool CompareJobsByName(Job* job1, Job* job2);
    static bool CompareJobsByPass(Job* job1, Job* job2);

    void FinishJob();
    void Interrupt();
    void Block();
    void UnBlock(const std::string name);
    void PrintRunnables();
    void PrintRunningTask();
    void PrintBlockedTasks();

    // Data Members
    std::map<std::string, Job*> idleJobs;
    std::map<std::string, Job*> blockedJobs;
    Job* currRunningJob;
    bool systemRunning;
};