#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 1000
#define MSG_SIZE 40

// Program entry point.
int main(int argc, char **argv)//argc=how many arguments, argv=what is each argument
{
  if (argc!=3)
  {
    perror("We need exactly 3 arguments");
  }
  else {
    int fd = open(argv[1], O_CREAT|O_RDWR|O_TRUNC, S_IRWXU);
    int n = atoi(argv[2]);//argv is an array of strings, but atoi changes a string to an int
    char buffer[BUFFER_SIZE];
    char msg[MSG_SIZE];
    pid_t pid = fork();
    if (pid <0)
    {
        perror("fork");
    }
//here is child 1
    else if (pid == 0)
    {
        printf("[Child1] Started. PID: %d, PPID: %d\n", getpid(), getppid());
        for (int k=0; k<=n; k=k+2)
      {
        time_t x = time(NULL);
        sprintf(msg,"Message from %d\n",getpid());
        printf("[Child1] Heartbeat PID: %d Time: %ld k: %d\n", getpid(), x, k);
        write(fd, msg, strlen(msg));
	sleep(1);
      }
	printf("[Child1] Terminating!\n");
        exit (0); //We exit because we don't want the child to fork as well
    }
    pid_t pid1 = fork();//Only the parent sees this
    if (pid1 <0)
    {
      perror("fork");
    }
//Here is child 2
    else if (pid1==0)
    {
        printf("[Child2] Started. PID: %d, PPID: %d\n", getpid(), getppid());
        for (int k=1; k<=n; k=k+2)
      {
        time_t y = time(NULL);
	sprintf(msg,"Message from %d\n",getpid());
        printf("[Child2] Heartbeat PID: %d Time: %ld k: %d\n", getpid(), y, k);
        write(fd, msg, strlen(msg));
	sleep(1);
      }
	printf("[Child2] Terminating!\n");
        exit (0); //We exit for the same reason (in case we have to fork again)
    }
//this is for the parent
    else
    {
	 for (int k=0; k<=n/2; k++)
      {
	sleep(1);//gia na ksekinhsoun prwta ta child1 kai child2
        time_t z = time(NULL);
	sprintf(msg,"Message from %d\n",getpid());
        printf("[Parent] Heartbeat PID: %d Time: %ld\n", getpid(), z);
        write(fd, msg, strlen(msg));
      }
	printf("[Parent] Waiting for Child\n");
	printf("Child with PID=%d terminated\n", wait(NULL));
	printf("[Parent] Waiting for Child\n");
	printf("Child with PID=%d terminated\n", wait(NULL));
	lseek(fd,0,SEEK_SET);
	read(fd, buffer, 1000);//Whatever is read, is stored in the buffer
	printf("%s",buffer);
	close(fd);
    }
  }
    return 0;
}
