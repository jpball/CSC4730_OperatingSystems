#include "Scheduler.hpp"

using namespace std;

Scheduler::Scheduler()
{
    systemRunning = false;
    currRunningJob = nullptr;
}
//--
Scheduler::~Scheduler()
{
    // Incase we have
    if(currRunningJob != nullptr)
    {
        delete currRunningJob;
        currRunningJob = nullptr;
    }
    // Incase we have idle jobs waiting to be deleted
    for(map<string, Job*>::iterator it = idleJobs.begin(); it != idleJobs.end(); it++)
    {
        delete it->second;
    }
    // Incase we have blocked jobs waiting to be deleted
    for(map<string, Job*>::iterator it = blockedJobs.begin(); it != blockedJobs.end(); it++)
    {
        delete it->second;
    }
}
//--
/*
    Read in the file via the path specified
    And execute the instruction found on each line
*/
void Scheduler::RunInstructionFile(const std::string filePath)
{
    ifstream FIN(filePath);
    if(!FIN.is_open()){
        fprintf(stderr, "ERROR: Instruction file path could not be opened (%s)\n", filePath.c_str());
        exit(1);
    }
    // File opened successfully
    string line;
    // Read each line from the file
    while(getline(FIN, line))
    {
        if(line != "" && line != "\n" && line != "\r")
        {
            RunInstructionString(line);
        }
    }
    FIN.close();
}
//--
/* 
    Given a line containing an instruction (i.e. "newjob,A,10")
    Separate the string into the appropriate details and use it
    to call RunCommand
 */
void Scheduler::RunInstructionString(std::string line)
{
    string opcode;
    string arg1;
    int arg2 = -1;

    size_t cIndex = line.find(',');
    if(cIndex != string::npos)
    {
        // Comma found
        opcode = line.substr(0, cIndex);
        line.erase(0, cIndex+1); // Erase up to (AND INCLUDING) first comma

         // More to the line exists
        if(line.size() > 0)
        {
            cIndex = line.find(','); // Find next comma
            if(cIndex != string::npos)
            {
                // Comma found
                arg1 = line.substr(0, cIndex); // Take up to that as the arg1
                line.erase(0, cIndex+1); // Erase up to next comma
                if(line.size() > 0)
                {
                    arg2 = stoi(line);
                }
            }
            else
            {
                arg1 = line;
            }
        }
    }
    else
    {
        // Comma not found
        opcode = line;
    }
    RunCommand(opcode, arg1, arg2);
}
//--
/* 
    Parses the given opcode and any potential arguments
Args:
    const string opcode --> The string representation of an instruction

Return:
    OPCODE --> The supplied opcode argument transformed into the proper ENUM
*/
OPCODE Scheduler::ParseCode(const string opcode)
{
    // Using the first letter of the opcode, 
    // we can go through a switch statement to determine the proper value
    switch (opcode.at(0))
    {
    case 'n':
    {
        if (opcode == "newjob")
        {
            return NEWJOB;
        }
        break;
    }

    case 'f':
    {
        if (opcode == "finish")
        {
            return FINISH;
        }
        break;
    }

    case 'i':
    {
        if (opcode == "interrupt")
        {
            return INTERRUPT;
        }
        break;
    }

    case 'b':
    {
        if (opcode == "block")
        {
            return BLOCK;
        }
        else if (opcode == "blocked")
        {
            return BLOCKED;
        }
        break;
    }
    case 'u':
    {
        if (opcode == "unblock")
        {
            return UNBLOCK;
        }
        break;
    }
    case 'r':
    {
        if (opcode == "running")
        {
            return RUNNING;
        }
        else if (opcode == "runnable")
        {
            return RUNNABLE;
        }
        break;
    }
    default:
    {
        break;
    }
    }
    fprintf(stderr, "ERROR: Invalid instruction found:%s\n", opcode.c_str());
    return INVALID;
}
//--
/*
Given the read in instruction & arguments, convert the instruction using ParseCode and run the desired instruction

Instruct List
    opcode	    argument 1  argument 2  meaning
    newjob	    NAME	    PRIORITY	A new job with specified PRIORITY and NAME has arrived
    finish			                    The currently running job has terminated - it is an error if the system is idle
    interrupt			                A timer interrupt has occurred - the currently running job's quantum is over
    block			                    The currently running job has become blocked
    unblock	    NAME		            The named job becomes unblocked - it is an error if it was not blocked
    runnable			                Print information about the jobs in the runnable queue
    running			                    Print information about the currently running job
    blocked			                    Print information about the jobs on the blocked queue
*/
void Scheduler::RunCommand(const string &opcode, const string &arg1 = "", const int &arg2 = -1)
{
    OPCODE oCode = ParseCode(opcode);
    switch (oCode)
    {
        case NEWJOB:
        {
            CreateNewJob(arg1, arg2);
            break;
        }
        case INTERRUPT:
        {
            Interrupt();
            break;
        }
        case BLOCK:
        {
            Block();
            break;
        }
        case UNBLOCK:
        {
            UnBlock(arg1);
            break;
        }
        case FINISH:
        {
            FinishJob();
            break;
        }
        case RUNNING:
        {
            PrintRunningTask();
            break;
        }
        case RUNNABLE:
        {
            PrintRunnables();
            break;
        }
        case BLOCKED:
        {
            PrintBlockedTasks();
            break;
        }
        case INVALID:
        default:
        {
            break;
        }
    }
}
//--
/*
    Used to sort our jobs alphebetically
    Used when we are scheduling ties
*/
bool Scheduler::CompareJobsByName(Job* job1, Job* job2)
{
    return (job1->name < job2->name);
}
//--
/*
    Used to sort our jobs by pass value
    Used when we are printing our runnables
*/
bool Scheduler::CompareJobsByPass(Job* job1, Job* job2)
{
    return (job1->pass < job2->pass);
}
//--
/*
    Go through each of the idle jobs and return whichever Job has the lowest pass
    If we have a tie, take whichever is first alphebetically by name 
*/
Scheduler::Job* Scheduler::GrabMinPassJob()
{
    list<Job*> lowestJobs;

    if(idleJobs.size() > 0)
    {
        int lowestPassVal = INT_MAX;
        for(map<string, Job*>::iterator it = idleJobs.begin(); it != idleJobs.end(); it++)
        {
            if(it->second->pass <= lowestPassVal)
            {
                if(it->second->pass < lowestPassVal)
                {
                    // Current Job we are looking at has a lower pass val
                    lowestJobs.clear();
                    lowestJobs.push_back(it->second);
                    lowestPassVal = it->second->pass; // Keep track of our lowest value so far
                }
                else
                {
                    // Pass value of iterated job is equal to the lowest
                    lowestJobs.push_back(it->second);
                }
            }
        }
        if(lowestJobs.size() > 1)
        {
            lowestJobs.sort(CompareJobsByName); // Sort lowest jobs alphabetically by name
        }
        return lowestJobs.front(); // Return the first job alphabetically
    }
    // Nothing in our idle list to consider
    return nullptr;
}
//--
/*
    OPCODE: newjob
    MEANING: A new job with specified PRIORITY and NAME has arrived

    A new job is entered into the system. 
    Its name and priority are given. 
    Assume all job names are unique. 
    A new job's arrival does not cause a rescheduling unless the system was idle.
*/
void Scheduler::CreateNewJob(const string name, const int& priority)
{
    idleJobs[name] = new Job(name, priority);
    printf("New job: %s added with priority: %d\n", name.c_str(), priority);

    if(!systemRunning)
    {
        Reschedule();
    }
}
//--
/*
    OPCODE: finish
    MEANING:    The currently running job has completed and should be removed from the system.
                If the system is idle, it is an error.
*/
void Scheduler::FinishJob()
{
    if(systemRunning)
    {
        Job cj = *currRunningJob;
        printf("Job: %s completed.\n", cj.name.c_str());
        delete currRunningJob;
        currRunningJob = nullptr;
        Reschedule();
    }
    else
    {
        printf("Error. System is idle.\n");
    }
}
//--
/*
    Find the lowest pass job and schedule it, sending our current running job to the idleJobs
*/
void Scheduler::Reschedule()
{
    if(idleJobs.size() > 0 || currRunningJob != nullptr)
    {
        Job* nextJob = GrabMinPassJob(); // Find the job with the smallest pass
        if(nextJob != nullptr)
        {
            // WE have a new job we need to schedule
            if(nextJob != currRunningJob)
            {
                // This job not currently running
                if(currRunningJob != nullptr)
                {
                    // WE have something already running
                    // Thus we need to swap
                    idleJobs[currRunningJob->name] = currRunningJob; // Add our currently schedule job back
                    currRunningJob = nextJob;
                    idleJobs.erase(nextJob->name);
                    systemRunning = true;
                }
                else
                {
                    // Nothing is running right now
                    systemRunning = true;
                    currRunningJob = nextJob;
                    idleJobs.erase(currRunningJob->name);
                }
            }
        }
        if(currRunningJob != nullptr)
        {
            // We have a job running
            printf("Job: %s scheduled.\n", currRunningJob->name.c_str());
        }
    }
    else
    {
        printf("System is idle.\n");
        systemRunning = false;
    }
}
/*
    OPCODE: interrupt
    MEANING:    The currently running task has completed its quantum. 
                     
    Adjust your bookkeeping. The scheduler needs to run again.
    It is an error if 'interrupt' is received when the system is idle.
*/
void Scheduler::Interrupt()
{
    if(systemRunning)
    {
        // If we were running something, increase its pass
        if(currRunningJob != nullptr)
        {
            currRunningJob->pass += currRunningJob->stride;
        }
        Reschedule(); // Run the next job
    }
    else
    {
        // System is IDLE
        printf("Error. System is idle.\n");
    }
}
//--
/*
    OPCODE: block
    MEANING: The currently running task has become blocked. Perhaps it is asking for an I/O.
    It is an error if the system is idle.
*/
void Scheduler::Block()
{
    if(systemRunning)
    {
        Job* bljb = currRunningJob; 
        blockedJobs[bljb->name] = bljb;
        printf("Job: %s blocked.\n", bljb->name.c_str());
        currRunningJob = nullptr;
        Reschedule();
    }
    else
    {
        // System is IDLE
        printf("Error. System is idle.\n");
    }
}
//--
/*
    OPCODE: unblock
    SYNTAX: unblock,A
    MEANING: The named job has become unblocked.

    It is an error if the named job was not blocked.
    Unblocked jobs return to the runnables. The scheduler is not run unless the system was idle.
*/
void Scheduler::UnBlock(const string name)
{
    if(blockedJobs.count(name) > 0)
    {
        // WE HAVE A BLOCKED JOB WITH THAT NAME
        Job* unblockedJob = blockedJobs[name]; // Grab that blocked job
        blockedJobs.erase(name); // Remove it from blocked jobs

        idleJobs[unblockedJob->name] = unblockedJob; // Move it into the idle jobs
        printf("Job: %s has unblocked. Pass set to: %d\n", unblockedJob->name.c_str(), unblockedJob->pass);
        // The scheduler is not run unless the system was idle.
        if(!systemRunning)
        {
            // System was IDLE
            Reschedule();
        }
    }
    else
    {
        // Job was not previously blocked!
        printf("Error. Job: %s not blocked.\n", name.c_str());
    }
}
//--
/*
    OPCODE: runnable
    MEANING: The runnables, if any, are listed. 
    Example:
        Runnable:
        NAME    STRIDE  PASS  PRI
        C       500     1000  200
        A       500     1500  200
    These must be listed in the order they would be scheduled.
*/
void Scheduler::PrintRunnables()
{
    list<Job*> allJobs;

    printf("Runnable:\n");
    if(idleJobs.size() > 0)
    {
        printf("NAME    STRIDE  PASS  PRI\n");
        for(map<string, Job*>::iterator it = idleJobs.begin(); it != idleJobs.end(); it++)
        {
            // For each job in our idleJobs map
            allJobs.push_back(it->second); // add it to our listed (used to sort later)
        }
        allJobs.sort(CompareJobsByPass); // Sort each idle job by its pass

        // Print out each job in the order it would be scheduled
        for(list<Job*>::iterator it = allJobs.begin(); it != allJobs.end(); it++)
        {
            printf("%-8s%-8d%-6d%-6d\n", (**it).name.c_str(), (**it).stride, (**it).pass, (**it).priority);
        }
    }
    else
    {
        printf("None\n");
    }
}
//--
/*
    OPCODE: running
    MEANING:    The running task is described (if system is not idle). 
        Example:
            Running:
            NAME    STRIDE  PASS  PRI
            B       1000    1000  100
*/
void Scheduler::PrintRunningTask()
{
    printf("Running:\n");
    if(currRunningJob != nullptr)
    {
        printf("NAME    STRIDE  PASS  PRI\n");
        printf("%-8s%-8d%-6d%-6d\n", currRunningJob->name.c_str(), currRunningJob->stride, currRunningJob->pass, currRunningJob->priority);
    }
    else
    {
        printf("None\n");
    }
}
//--
/*
    OPCODE: blocked
    MEANING: The blocked tasks are listed, if any. 
        Example:
            Blocked:
            NAME    STRIDE  PASS  PRI
            A       500     2000  200
*/
void Scheduler::PrintBlockedTasks()
{
    printf("Blocked:\n");
    if(blockedJobs.size() > 0)
    {
        printf("NAME    STRIDE  PASS  PRI\n");
        for(map<string, Job*>::iterator it = blockedJobs.begin(); it != blockedJobs.end(); it++){
            printf("%-8s%-8d%-6d%-6d\n", it->second->name.c_str(), it->second->stride, it->second->pass, it->second->priority);
        }
    }
    else
    {
        printf("None\n");
    }

}
//--
