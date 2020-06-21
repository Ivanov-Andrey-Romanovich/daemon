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
#include <semaphore.h>
#include <pthread.h>

sem_t smphr;


bool signal1 = false;
bool signal2 = false;
bool terminated = false;
bool child = false;

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

void ChildAlarm(int sign){
	child = true;
}

int RunCommands(char **Argv){
	int outFile2 = open("commandsoutp.txt", O_CREAT | O_RDWR | O_APPEND, S_IRWXU);
	dup2(outFile2, STDOUT_FILENO);
	close(outFile2);
	execv(Argv[0],Argv);
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
	signal(SIGCHLD, ChildAlarm);
	sem_init(&smphr,0,1);
	int cnt, stat;
	char commands[64256];
	int commandFile = open(filename, O_RDWR, S_IRWXU);
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
			cnt = read(commandFile, commands, sizeof(commands));
			close(commandFile);
			if (cnt > 0){
				int comCnt = 0;
				char* tmp[10];
				char* command;
				command = strtok(commands, "\n");
				while (command != NULL){
					tmp[comCnt++] = command;
					command = strtok(NULL, "\n");
				}
				for(int i = 0;i<comCnt;i++){
					pid_t chpid;
					if(chpid=fork() == 0){
						int Argc = 0;
						char *Argv[10];
						char *arg;
						arg = strtok(tmp[i]," ");
						Argv[Argc++] = arg;
						while(arg != NULL){
							arg = strtok(NULL, " ");
							Argv[Argc++] = arg;
						}
						Argv[Argc] = NULL;
						int wait = sem_wait(&smphr);
						if(wait == -1){
							write(outFile, "sem_wait error\n", 16);
						} else{
							char msg3[100] = "\nRun command: ";
							strcat(msg3,Argv[0]);
							write(outFile, msg3, 15 + strlen(Argv[0]));
							RunCommands(Argv);
						}
					}
					else if(chpid > 0){
						while (1){
							if(child){
								waitpid(-1, NULL, 0);
								sem_post(&smphr);
								child = false;
								break;
							}
							pause();
						}
					} else if(chpid<0){
						write(outFile,"\nFork error\n",12);
					}
				}
			}
			signal2 = false;
		}
		pause();
	}
	sem_destroy(&smphr);
	write(outFile, end, sizeof(end));
	close(outFile);
	exit(0);
}

int main(int argc, char *argv[])
{
	int daemstat;
	pid_t parpid;
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
		
		
