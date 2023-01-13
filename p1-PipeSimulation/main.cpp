#include "CommandOptions.hpp"
#include <stdio.h>

using namespace std;

#define RD_SIDE 0
#define WT_SIDE 1

int main(int argc, char *argv[])
{
	CommandOptions copt(argc, argv);
	if (copt.IsDEBUG()){
		copt.DEBUG_PrintOptionValues();
	}

	if (copt.GetPSecExecPath() == nullptr)
	{
		// No second prog given
		// Thus no need for a pipe
		pid_t pid;
		if (!(pid = fork()))
		{
			// CHILD IS HERE
			if (copt.GetPNewDirPath() != nullptr)
			{
				int fd;
				// Check if valid directory
				if ((fd = open(copt.GetPNewDirPath()->c_str(), O_DIRECTORY)) < 0)
				{
					perror("Directory Path Error");
					return (1);
				}
				else
				{
					close(fd);
					// New directory is valid
					chdir(copt.GetPNewDirPath()->c_str());
				}
			}
			if (copt.GetPInPath() != nullptr)
			{
				// We have a new input
				if (copt.IsDEBUG()) { fprintf(stderr, "DEBUG -- Opening INPUT at: %s\n", copt.GetPInPath()->c_str()); }
				int inFD = open(copt.GetPInPath()->c_str(), O_RDONLY);
				if (inFD < 0)
				{
					// File wasn't opened
					perror("Could not open redirected input file:");
					exit(1);
				}
				else
				{
					// File opened
					close(STDIN_FILENO);
					dup2(inFD, STDIN_FILENO); // Opens in STDIN's place and closes original FD
				}
			}

			if (copt.GetPOutPath() != nullptr)
			{
				// We have a new output
				int outFD = open(copt.GetPOutPath()->c_str(), O_WRONLY | O_CREAT | (!copt.IsOverwriting() ? (O_APPEND) : (O_TRUNC)), S_IWUSR | S_IRUSR);
				if (outFD < 0)
				{
					// File wasn't opened
					perror("Could not open redirected output file:");
					exit(1);
				}
				else
				{
					// File opened
					close(STDOUT_FILENO);
					dup2(outFD, STDOUT_FILENO); // Opens in STDIN's place and closes original FD
				}
			}

			char *childArgV[2];
			childArgV[0] = strdup(copt.GetPFirstExecPath()->c_str());
			childArgV[1] = nullptr;

			if (execvp(childArgV[0], childArgV) == -1)
			{
				perror("Prog 1 exec failed");
				exit(1);
			}
		}

		// PARENT
		int status;
		waitpid(pid, &status, 0);
		printf("Child 1: %d returns %d\n", pid, WEXITSTATUS(status));
	}
	else
	{
		// We have a second prog
		// SO WE NEED A PIPE
		int pipe_fds[2];

		if (pipe(pipe_fds))
		{
			perror("Pipe Error:");
			return (5);
		}

		/*
		CHILD #1 
			Exec's -1
			Uses -i as input
			Sends data to pipe_fds[WT_SIDE]
			Does not read
		*/
		pid_t pid1;
		if (!(pid1 = fork()))
		{
			if (copt.IsDEBUG())
			{
				fprintf(stderr, "Child #1 is running!\n");
			}
			if (copt.GetPNewDirPath() != nullptr)
			{
				int fd;
				// Check if valid directory
				if ((fd = open(copt.GetPNewDirPath()->c_str(), O_DIRECTORY)) < 0)
				{
					perror("Directory Path Error");
					return (1);
				}
				else
				{
					// New directory is valid
					close(fd);
					chdir(copt.GetPNewDirPath()->c_str());
				}
			}
			// CHILD 1 IS HERE
			if (copt.GetPInPath() != nullptr)
			{
				if (copt.IsDEBUG())
				{
					fprintf(stderr, "DEBUG -- C1 has inpath\n");
				}
				// We have a new input
				int inFD = open(copt.GetPInPath()->c_str(), O_RDONLY);
				if (inFD < 0)
				{
					// File wasn't opened
					perror("Could not open redirected input file:");
					exit(1);
				}
				else
				{
					// File opened
					close(STDIN_FILENO);
					dup2(inFD, STDIN_FILENO); // Opens in STDIN's place and closes original FD
				}
			}
			else
			{
				if (copt.IsDEBUG())
				{
					fprintf(stderr, "DEBUG -- C1 has no inpath\n");
				}
				close(pipe_fds[RD_SIDE]);
			}

			close(STDOUT_FILENO);
			dup2(pipe_fds[WT_SIDE], STDOUT_FILENO);

			char *childArgV[2];
			childArgV[0] = strdup(copt.GetPFirstExecPath()->c_str());
			childArgV[1] = nullptr;

			if (copt.IsDEBUG())
			{
				fprintf(stderr, "DEBUG -- C1 attempting to exec %s\n", childArgV[0]);
			}

			if (execvp(childArgV[0], childArgV) == -1)
			{
				perror("Error when executing first program:");
				exit(1);
			}
		}

		/*
		CHILD #2
		*/
		pid_t pid2;
		if (!(pid2 = fork()))
		{
			if (copt.GetPNewDirPath() != nullptr)
			{
				// Check if valid directory
				int fd;
				if ((fd = open(copt.GetPNewDirPath()->c_str(), O_DIRECTORY)) < 0)
				{
					perror("Directory Path Error");
					return (1);
				}
				else
				{
					close(fd);
					// New directory is valid
					chdir(copt.GetPNewDirPath()->c_str());
				}
			}
			if (copt.IsDEBUG())
			{
				fprintf(stderr, "Child #2 is running!\n");
			}

			// CHILD 2 IS HERE
			if (copt.GetPOutPath() != nullptr)
			{
				if (copt.IsDEBUG()) { fprintf(stderr, "DEBUG -- C2 has outpath\n"); }
				// We have a new output
				// Attempt opening
				int outFD = open(copt.GetPOutPath()->c_str(), O_WRONLY | (!copt.IsOverwriting() ? (O_APPEND | O_CREAT) : (O_CREAT)), S_IWUSR | S_IRUSR);
				if (outFD < 0)
				{
					// File wasn't opened
					perror("Could not open redirected output file:");
					exit(1);
				}
				else
				{
					// File opened
					close(STDOUT_FILENO);
					dup2(outFD, STDOUT_FILENO); // Opens in STDIN's place and closes original FD
				}
			}
			else
			{
				if (copt.IsDEBUG())
				{
					fprintf(stderr, "DEBUG -- C2 has no outpath\n");
				}
			}
			close(STDIN_FILENO);
			dup2(pipe_fds[RD_SIDE], STDIN_FILENO);
			close(pipe_fds[WT_SIDE]);

			char *childArgV[2];
			childArgV[0] = strdup(copt.GetPSecExecPath()->c_str());
			childArgV[1] = nullptr;

			if (copt.IsDEBUG()) { fprintf(stderr, "DEBUG -- C2 attempting to exec %s\n", childArgV[0]); }
			if (execvp(childArgV[0], childArgV) == -1)
			{
				perror("Error when executing second program:");
				exit(1);
			}
		}

		// Close both sides of pipe as PARENT
		close(pipe_fds[RD_SIDE]);
		close(pipe_fds[WT_SIDE]);

		// PARENT
		// Wait for both children to end (individually) and print out a message
		int status;
		waitpid(pid2, &status, 0);
		printf("Child 2: %d returns %d\n", pid2, WEXITSTATUS(status));

		waitpid(pid1, &status, 0);
		printf("Child 1: %d returns %d\n", pid1, WEXITSTATUS(status));
	}
	return 0;
}
