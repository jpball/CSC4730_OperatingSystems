#include "MLFQSch.hpp"

using namespace std;

MLFQSch::MLFQSch()
{
    systemRunning = false;
    currRunningJob = nullptr;

    for (size_t queue = 0; queue < NUMBER_OF_QUEUES; queue++)
    {
        FeedbackQueue *fbq = new FeedbackQueue();
        allQueues.push_back(fbq);
    }
}
//--
MLFQSch::~MLFQSch()
{
    // Incase we have
    if (currRunningJob != nullptr)
    {
        delete currRunningJob;
        currRunningJob = nullptr;
    }

    // Delete all of the queues
    for (auto que : allQueues)
    {
        // Check for runnables to delete
        if (que->runnables.size() > 0)
        {
            for (auto job : que->runnables)
            {
                delete job;
            }
        }

        // Check for blocked jobs to delete
        if (que->blocked.size() > 0)
        {
            for (auto job : que->runnables)
            {
                delete job;
            }
        }

        delete que; // Delete the actual queue
    }
}
//--
/*
    Read in the file via the path specified
    And execute the instruction found on each line
*/
void MLFQSch::RunInstructionFile(const std::string &filePath)
{
    ifstream FIN(filePath);
    if (!FIN.is_open())
    {
        fprintf(stderr, "Input file failed to open.\n");
        exit(1);
    }
    // File opened successfully
    string line;
    // Read each line from the file
    while (getline(FIN, line))
    {
        if (line != "" && line != "\n" && line != "\r")
        {
            RunInstructionString(line);
        }
    }
    FIN.close();
}
//--
/*
    Given a line containing an instruction (i.e. "newjob,A")
    Separate the string into the appropriate details and use it
    to call RunCommand
 */
void MLFQSch::RunInstructionString(std::string line)
{
    stringstream SIN(line);
    if (SIN.good())
    {
        string opcode;
        string arg1;

        getline(SIN, opcode, ','); // Read the opcode
        if (line.size() > 0)
        {
            if (line.find(',') != string::npos)
            {
                // We found a comma
                getline(SIN, arg1, ',');
            }
            else
            {
                getline(SIN, arg1);
            }
        }

        RunCommand(opcode, arg1);
    }
}
//--
/*
    Parses the given opcode and any potential arguments
Args:
    const string opcode --> The string representation of an instruction

Return:
    OPCODE --> The supplied opcode argument transformed into the proper ENUM
*/
OPCODE MLFQSch::ParseCode(const string &opcode)
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
    case 'e':
    {
        if (opcode == "epoch")
        {
            return EPOCH;
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
Given the read in instruction & argument, convert the instruction using ParseCode and run the desired instruction

Instruct List
    opcode	    argument 1      meaning
    newjob	    NAME	  	    A new job arrives with the given NAME
    finish			            The currently running job has terminated - it is an error if the system is idlee
    interrupt			        A timer interrupt has occurred - the currently running job's quantum is over
    block			            The currently running job has become blocked
    unblock	    NAME            The named job becomes unblocked - it is an error if it was not blocked
    runnable			        Print information about the jobs in the runnable queue
    running			            Print information about the currently running job
    blocked			            Print information about the jobs on the blocked queue
    epoch                       An Epoch has elapsed. Process as per MLFQ algorithm
*/
void MLFQSch::RunCommand(const string &opcode, const string &arg1 = "")
{
    OPCODE oCode = ParseCode(opcode);
    switch (oCode)
    {
    case NEWJOB:
    {
        CreateNewJob(arg1);
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
    case EPOCH:
    {
        HandleEpoch();
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
    You will be told when an epoch has expired when you receive an "epoch" command.
    Handle this as per the MLFQ algorithm.

    Note that "epoch" does not cause a rescheduling.

    That is, a process running when you receive an "epoch"
        will remain running but will also be marked as coming
        from the highest priority queue.

    Rule 5: After some time period S,
        move all the jobs in the system to the topmost queue.

    Runnables, blocked, running
*/
void MLFQSch::HandleEpoch()
{
    for (uint32_t queueIndex = 1; queueIndex < allQueues.size(); queueIndex++)
    {
        // ^^ Skipping the first queue, go through the rest
        FeedbackQueue *fbq = allQueues.at(queueIndex);
        // vv The queue we are looking at has some runnables to move up
        if (fbq->runnables.size() > 0)
        {
            while (fbq->runnables.size() > 0)
            {
                fbq->runnables.front()->priority = 0;                         // Mark the priority as 0
                allQueues.at(0)->runnables.push_back(fbq->runnables.front()); // Take the front job and move it to the top FBQ
                printf("Job: %s lifted up.\n", fbq->runnables.front()->name.c_str());
                fbq->runnables.pop_front(); // Remove it from the runnables
            }
        }
        // The queue we are looking at has some blocked jobs to move
        if (fbq->blocked.size() > 0)
        {
            while (fbq->blocked.size() > 0)
            {
                fbq->blocked.front()->priority = 0;                       // Mark the priority as 0
                allQueues.at(0)->blocked.push_back(fbq->blocked.front()); // Take the front job and move it to the top FBQ
                printf("Job: %s lifted up.\n", fbq->blocked.front()->name.c_str());
                fbq->blocked.pop_front(); // Remove it from the runnables
            }
        }
    }
    if (currRunningJob)
    {
        currRunningJob->priority = 0;
        printf("Job: %s lifted up.\n", currRunningJob->name.c_str());
    }
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
void MLFQSch::CreateNewJob(const string &name)
{
    FeedbackQueue *highestQueue = allQueues.at(0);

    // Create Job on heap
    // Job(std::string n, uint32_t p) { name = n;  priority = p; }
    Job *nJob = new Job(name, 0);
    highestQueue->runnables.push_back(nJob);

    printf("New job: %s added.\n", name.c_str());

    if (systemRunning == false)
    {
        // System not running previously
        ScheduleNextJob();
    }
}
//--
/*
    OPCODE: finish
    MEANING:    The currently running job has completed and should be removed from the system.
                If the system is idle, it is an error.
*/
void MLFQSch::FinishJob()
{
    if (systemRunning)
    {
        printf("Job: %s completed.\n", currRunningJob->name.c_str());
        delete currRunningJob;
        currRunningJob = nullptr;
        ScheduleNextJob();
    }
    else
    {
        printf("Error. System is idle.\n");
    }
}
//--
/*
    Use the MLFQ scheduling algorithm

*/
void MLFQSch::ScheduleNextJob()
{

    u_int16_t hqI = 0;
    FeedbackQueue *highestQueue = allQueues.at(hqI);
    bool somethingToRun = false;

    for (hqI = 0; hqI < allQueues.size(); hqI++)
    {
        // For each of our queues
        // If the one we are looking at has something to run
        // Let's keep track of it
        if (allQueues.at(hqI)->runnables.size() > 0)
        {
            somethingToRun = true;
            highestQueue = allQueues.at(hqI);
            break;
        }
    }

    if (somethingToRun)
    {
        // Now we have some runnable at our current highestQueue
        if (!systemRunning)
        {
            systemRunning = true;
        }

        // Find the next highest job in the highest queue and run it
        currRunningJob = highestQueue->runnables.front();
        highestQueue->runnables.pop_front();
        printf("Job: %s scheduled.\n", currRunningJob->name.c_str());
    }
    else
    {
        // We have nothing to run, thus we become idle
        systemRunning = false;
        printf("System is idle.\n");
    }
}
/*
    OPCODE: interrupt
    MEANING:    The currently running task has completed its quantum.

    Adjust your bookkeeping. The scheduler needs to run again.
    It is an error if 'interrupt' is received when the system is idle.

    Rule 4: Once a job uses up its time allotment at a
    given level (regardless of how many times it has given up the CPU), its priority is
    reduced (i.e., it moves down one queue)
*/
void MLFQSch::Interrupt()
{
    if (systemRunning)
    {
        if (currRunningJob != nullptr)
        {
            // Add our currently running job back into the runnables queue
            // OF THE NEXT LOWEST QUEUE
            if (currRunningJob->priority < allQueues.size() - 1)
            {
                // WE are at the second to last (or higher) queue
                currRunningJob->priority++;
                allQueues.at(currRunningJob->priority)->runnables.push_back(currRunningJob);
                currRunningJob = nullptr;
            }
            else
            {
                // WE are at the highest queue
                allQueues.at(currRunningJob->priority)->runnables.push_back(currRunningJob);
                currRunningJob = nullptr;
            }
        }
        ScheduleNextJob(); // Run the next job
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
void MLFQSch::Block()
{
    if (systemRunning)
    {
        Job *bljb = currRunningJob;
        allQueues.at(bljb->priority)->blocked.push_back(bljb); // Add the job to the blocked in the queue
        printf("Job: %s blocked.\n", bljb->name.c_str());
        currRunningJob = nullptr;
        ScheduleNextJob();
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
void MLFQSch::UnBlock(const string &name)
{
    bool jobWasUnBlocked = false;
    Job *jobToUnBlock = nullptr;

    for (FeedbackQueue *queue : allQueues)
    {
        // Go through each of our queues
        for (size_t i = 0; i < queue->blocked.size(); i++)
        {
            // Go through each of the blocked jobs in the queue's blocked vector
            if (queue->blocked.at(i)->name == name)
            {
                // This job is the one we want to unblock
                jobWasUnBlocked = true;
                jobToUnBlock = queue->blocked.at(i);

                queue->runnables.push_front(jobToUnBlock);
                queue->blocked.erase(queue->blocked.begin() + i); // Erase the blocked job from the blocked jobs vector
                break;
            }
        }
        if (jobWasUnBlocked)
        {
            break;
        }
    }

    if (jobWasUnBlocked)
    {
        printf("Job: %s has unblocked.\n", jobToUnBlock->name.c_str());
        if(currRunningJob != nullptr)
        {
            if(jobToUnBlock->priority < currRunningJob->priority)
            {
                // Our jobtounblock has a BETTER priority and needs to be the one running
                // Add our currently running job back to it's original queue
                allQueues.at(currRunningJob->priority)->runnables.push_back(currRunningJob);
                currRunningJob = nullptr;
            }
        }
        ScheduleNextJob();
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
        Runnables:
        NAME    QUEUE
        B       0
        A       0
        C       2
        Z       3
        G       3
        T       3
    These must be listed in the order they would be scheduled.
*/
void MLFQSch::PrintRunnables()
{
    printf("Runnables:\n");
    bool headingPrinted = false;
    for (auto queue : allQueues)
    {
        // For each of our queues
        // GO through each job if they have any
        // And print out the info
        if (queue->runnables.size() > 0)
        {
            if (headingPrinted == false)
            {
                printf("NAME    QUEUE   \n");
                headingPrinted = true;
            }
            // We have items in our runnables in the indexed queue to print out
            for (Job *idleJob : queue->runnables)
            {
                printf("%-8s%-8u\n", idleJob->name.c_str(), idleJob->priority);
            }
        }
    }

    if (!headingPrinted)
    {
        // Heading wasn't printed thus we had nothing to print out
        printf("None\n");
    }
}
//--
/*
    OPCODE: running
    MEANING:    The running task is described (if system is not idle).
        Example:
            Running:
            NAME    QUEUE
            H       0
*/
void MLFQSch::PrintRunningTask()
{
    printf("Running:\n");
    if (currRunningJob != nullptr)
    {
        printf("NAME    QUEUE   \n");
        printf("%-8s%-8u\n", currRunningJob->name.c_str(), currRunningJob->priority);
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
            NAME    QUEUE
            I       0
            K       2
            J       2

*/
void MLFQSch::PrintBlockedTasks()
{
    printf("Blocked:\n");
    bool headingPrinted = false;
    for (auto queue : allQueues)
    {
        // For each of our queues
        // GO through each job if they have any
        // And print out the info
        if (queue->blocked.size() > 0)
        {
            if (headingPrinted == false)
            {
                printf("NAME    QUEUE   \n");
                headingPrinted = true;
            }
            // We have items in our runnables in the indexed queue to print out
            for (Job *idleJob : queue->blocked)
            {
                printf("%-8s%-8u\n", idleJob->name.c_str(), idleJob->priority);
            }
        }
    }

    if (!headingPrinted)
    {
        // Heading wasn't printed thus we had nothing to print out
        printf("None\n");
    }
}
//--
