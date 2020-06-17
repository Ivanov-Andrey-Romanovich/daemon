#include <stdio.h> 
#include <string.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <resolv.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool signal1 = false;
bool signal2 = false;
bool terminated = false;

void firstAlarm(int signum)
{
	signal1 = true;
}

void secondAlarm(int signum)
{
	signal2 = true;
}

void isTerminated(int signum)
{
	terminated = true;
}

int Daemon(char *filename)
{
	int outFile = open("outp.txt", O_CREAT | O_RDWR, S_IRWXU);
	char mes1[] = "Signal SIGUSR1 was cuptured!\n";
	char mes2[] = "Signal SIGUSR2 was cuptured!\n";
	char start[] = "Programm started!\n";
	char end[] = "Work is finished!\n ";

	signal(SIGUSR1, firstAlarm);
	signal(SIGUSR2, secondAlarm);
	signal(SIGTERM, isTerminated);

	char commands[64256];
	int commandFile = open(filename, O_RDWR, S_IRWXU);
	read(commandFile, commands, sizeof(commands));
	char *Argv[] = {NULL, NULL};
	Argv[0] = commands;
	int pid;
	dup2(commandFile, STDOUT_FILENO);
	close(commandFile);
	write(outFile, start, sizeof(start));
	while (!terminated)
	{
		if (signal1)
		{
			write(outFile, mes1, sizeof(mes1));
			signal1 = false;
		}
		if (signal2)
		{
			write(outFile, mes2, sizeof(mes2));
			write(outFile, end, sizeof(mes2));
			signal2 = false;
			execve("/bin/ls", Argv, NULL);
			/*pid = fork();
			if (pid < 0)
			{
				printf("can't fork\n");
				exit(EXIT_FAILURE);
			} else if (pid >= 0){
				execve("/bin/ls", Argv, NULL);
			}*/
		}
		sleep(100);
	}
	write(outFile, end, sizeof(end));
	close(outFile);
	exit(0);
}

int main(int argc, char *argv[])
{
	int daemstat;
	pid_t parpid;
	char buf[64256];
	parpid = fork();
	if(parpid < 0){
		printf("can't fork\n"); 
           	exit(1);    
	}            
        else if (parpid != 0)
         	exit(0);
	setsid();
	parpid = fork();

	if (parpid < 0){
		printf("can't fork\n");
		exit(1);
	}

	if (parpid != 0)
		exit(0);
	umask(0);
	pid_t childpid = getpid();
	printf("cat outp.txt - file should be empty. \n");
	printf("Pid of the daemon : %i .\n", childpid);
	printf("kill -SIGUSR1 %d - command to sent first signal to daemon.\n", childpid);
	printf("kill -SIGUSR2 %d - command to sent second signal to daemon,what write content of directory, and to stop the programm.\n", childpid);
	printf("kill %d - stop the programm.\n", childpid);
	printf("cat outp.txt - command to see changes in logs.\n");
	close(STDIN_FILENO);
	close(STDERR_FILENO);
	daemstat = Daemon(argv[1]);
	return daemstat;
}
		
		
