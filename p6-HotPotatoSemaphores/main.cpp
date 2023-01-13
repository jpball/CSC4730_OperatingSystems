/*
=============================
CSC 4730 - Operating Systems
Proj. 6 - Hot Potato

Created by Jordan Ball
=============================
*/

#include <iostream>
#include <pthread.h>
#include <getopt.h>
#include <cinttypes>
#include <vector>
#include <cassert>

#define E_USAGE_MENU_PRINTED 1
#define E_NUMBER_PLAYERS_INVALID 2
#define E_NUMBER_POTATOES_INVALID 3
#define DISPLAY_SLEEP_TIME_uS 250000

struct Sem
{
    int32_t value;
    pthread_cond_t cond;
    pthread_mutex_t lock;
};

/*
    ===== FUNCTION PROTOTYPES =====
*/
// Project Functions
void SemInit(Sem &s, int32_t initial_value);
void SemPost(Sem &s);
int32_t SemWait(Sem &s);
void *player(void *arg);
float CalcVariance();

// Utility Functions
bool HandleOptions(int argc, char *argv[]);
void PrintUsage();
inline void ValidateNumPlayers();
inline void ValidateNumPotatoes();

/*
========== GLOBAL VARIABLES
    Able to be accessed by all threads
*/
uint16_t numPlayers;
uint16_t numPotatoes;

// The possession vector of bool has a true in the
// i'th position only when the i'th player is in possession of the potato.
std::vector<bool> *p_possessionVec;

// The counters vector holds the number of times each player (thread) has held the potato.
std::vector<uint64_t> *p_counterVec;

// Holds our threads
std::vector<pthread_t> *p_threadVec;

Sem semaphore;
pthread_mutex_t pauseLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pauseCond = PTHREAD_COND_INITIALIZER;

bool canRun = false;

int main(int argc, char *argv[])
{
    // Default values
    numPlayers = 4;
    numPotatoes = 1;

    if (!HandleOptions(argc, argv))
    {
        PrintUsage();
        exit(E_USAGE_MENU_PRINTED);
    }

    // Validate options
    ValidateNumPlayers();
    printf("Number of children: %d\n", numPlayers);

    ValidateNumPotatoes();
    printf("Number of potatoes:  %d\n", numPotatoes);

    // Initialize our vectors
    p_counterVec = new std::vector<uint64_t>(numPlayers, 0);
    p_possessionVec = new std::vector<bool>(numPlayers, false);
    p_threadVec = new std::vector<pthread_t>(numPlayers);

    // Initialize the semaphore value
    SemInit(semaphore, numPotatoes);

    // Set up and create the players based on numPlayers
    for (size_t i = 0; i < numPlayers; i++)
    {
        pthread_create(&(p_threadVec->at(i)), NULL, player, (void *)i);
    }

    usleep(numPlayers * 20);
    printf("Hit return to put potatoes in play: ");
    getchar();

    pthread_mutex_lock(&pauseLock);
    canRun = true;
    pthread_cond_broadcast(&pauseCond); // Wake up all of the sleeping threads so they can start playing
    pthread_mutex_unlock(&pauseLock);

    while (true)
    {

        pthread_mutex_lock(&pauseLock);
        // Print out the data to the screen
        for (size_t i = 0; i < p_counterVec->size(); i++)
        {
            printf("%c%llu ", p_possessionVec->at(i) ? '*' : ' ', p_counterVec->at(i));
        }
        printf("variance: %.2f\n", CalcVariance());
        assert(semaphore.value <= numPotatoes); // ASSERT THAT OUR SEMAPHORE NEVER GETS TOO LARGE
        pthread_mutex_unlock(&pauseLock);

        usleep(DISPLAY_SLEEP_TIME_uS);
        // Clear it
        system("clear");
    }

    return 0;
}
//--
/*
========================
    PROJECT FUNCTIONS
========================
*/
void *player(void *arg)
{
    size_t index = (size_t)(arg);

    printf("Child:  %zu has started\n", index);

    pthread_mutex_lock(&pauseLock);
    while (canRun == false)
    {
        pthread_cond_wait(&pauseCond, &pauseLock);
    }
    pthread_mutex_unlock(&pauseLock);

    while (true)
    {
        int32_t res(SemWait(semaphore));
        assert(res <= numPotatoes);

        // We have a potato
        p_possessionVec->at(index) = true;
        (p_counterVec->at(index))++;

        // Sleep for between 1,000 and 50,000 microseconds
        usleep((rand() % 49000) + 1000);

        p_possessionVec->at(index) = false; // Give up the potato

        SemPost(semaphore); // Place the potato back into the pool

        // Sleep for between 1,000 and 50,000 microseconds
        usleep((rand() % 49000) + 1000);
        assert(semaphore.value <= numPotatoes);
    }

    return NULL;
}
//--
void SemInit(Sem &s, int32_t initial_value)
{
    s.value = initial_value;
    s.cond = PTHREAD_COND_INITIALIZER;
    s.lock = PTHREAD_MUTEX_INITIALIZER;
}
//--
/*
    increment the value of semaphore s by one
    if there are one or more threads waiting, wake one
*/
void SemPost(Sem &s)
{
    pthread_mutex_lock(&s.lock);
    s.value++; // Add another potato to the pool
    assert(s.value <= numPotatoes); // ensure we never have too many potatoes in the pool
    pthread_cond_signal(&s.cond); // Wake up someone waiting in SemWait
    pthread_mutex_unlock(&s.lock);
}
//--
/*
    decrement the value of semaphore s by one
    wait if value of semaphore s is negative
*/
int32_t SemWait(Sem &s)
{
    pthread_mutex_lock(&s.lock);
    // wait if value of semaphore s is negative
    while (s.value <= 0)
    {
        pthread_cond_wait(&s.cond, &s.lock); // Wait because there's nothing left in the pool
    }
    s.value--; // We are not proceeding in order to use a potato so there will be one less in the pool
    pthread_mutex_unlock(&s.lock);
    return s.value;
}
//--
float CalcVariance()
{
    uint64_t largest = 0;
    uint64_t smallest = -1; // Force overflow with -1 initialization

    for (size_t i = 0; i < p_counterVec->size(); i++)
    {
        uint64_t val = p_counterVec->at(i);
        if (val > largest)
        {
            largest = val;
        }
        if (val < smallest)
        {
            smallest = val;
        }
    }

    /*
    The "variance" is the largest value minus the
    smallest value divided by the largest value times 100.
    Over time, as the other numbers are grow, the variance
    will get smaller.
    If it does not get smaller over time, you have a bug.
    */
    return ((largest - smallest) / (largest * 1.0)) * 100.0;
}
//--
/*
========================
    UTILITY FUNCTIONS
========================
*/
//--
void PrintUsage()
{
    printf("---- Usage ----\n");
    printf("-h               prints this\n");
    printf("-n numchildren   sets the number of children - default is 4\n");
    printf("-p numpotatoes   sets the number of potatoes - default is 1\n");
}
//--
/*
    -h to print help / usage.

    -n to set the number of players. The value defaults to 4. It cannot be less than 1 or more than 32.

    -p to set the number of potatoes. The value defaults to 1. It cannot be less than 1 nor more than the number of players.
*/
bool HandleOptions(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "hn:p:")) != -1)
    {
        switch (c)
        {
        default:
        case ('h'):
        {
            // Forces the HandleOptions to 'fail'
            // main() has us printing the usage menu upon returning false
            return false;
        }
        case ('n'):
        {
            // -n to set the number of players. The value defaults to 4. It cannot be less than 1 or more than 32.
            numPlayers = atoi(optarg);
            break;
        }
        case ('p'):
        {
            // -p to set the number of potatoes. The value defaults to 1. It cannot be less than 1 nor more than the number of players.
            numPotatoes = atoi(optarg);
            break;
        }
        }
    }
    return true;
}
//--
inline void ValidateNumPotatoes()
{
    /*
        -p to set the number of potatoes.
        The value defaults to 1.
        It cannot be less than 1 nor more than the number of players.
    */
    if (numPotatoes < 1 || numPotatoes > numPlayers)
    {
        fprintf(stderr, "ERROR: Number of Potatoes [%d] is invalid.\n", numPotatoes);
        exit(E_NUMBER_POTATOES_INVALID);
    }
}
//--
inline void ValidateNumPlayers()
{
    /*
        -n to set the number of players.
        The value defaults to 4.
        It cannot be less than 1 or more than 32.
    */
    if (numPlayers < 1 || numPlayers > 32)
    {
        fprintf(stderr, "ERROR: Number of Players [%d] is invalid.\n", numPlayers);
        exit(E_NUMBER_PLAYERS_INVALID);
    }
}