#pragma once
#include <stdio.h>
#include <string>
#include <fstream>
#include <deque>
#include <vector>
#include <sstream>

#define NUMBER_OF_QUEUES 4

enum OPCODE {INVALID, NEWJOB, FINISH, INTERRUPT, BLOCK, UNBLOCK, RUNNABLE, RUNNING, BLOCKED, EPOCH};

class MLFQSch{
public:
    MLFQSch();
    ~MLFQSch();
    void RunInstructionFile(const std::string& filePath);
    void RunInstructionString(std::string line);
    void RunCommand(const std::string& com, const std::string& arg1);
private:
    struct Job
    {
        Job(const std::string& n, const uint32_t& p) : name(n), priority(p) {}

        std::string name;
        uint32_t priority; // Represents the queue number they are in
    };

    struct FeedbackQueue
    {
        std::deque<Job*> runnables;
        std::deque<Job*> blocked;
    };



    // Methods
    OPCODE ParseCode(const std::string& code);
    void CreateNewJob(const std::string& name = "");
    void ScheduleNextJob();

    void HandleEpoch();
    void FinishJob();
    void Interrupt();
    void Block();
    void UnBlock(const std::string& name);
    void PrintRunnables();
    void PrintRunningTask();
    void PrintBlockedTasks();

    // Data Members
    std::vector<FeedbackQueue*> allQueues;
    Job* currRunningJob;
    bool systemRunning;
};