#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>


int volatile n, k[100];//volatile is used to prevent the compiler from applying any optimizations on objects
int i, status;
pid_t pid, signalID;
bool volatile sigusr1 = 0, newChild = 0, state = 1, ignite = 0;
char *str;


	void sigusr1_handler(int signum) {
		sigusr1 = !sigusr1;
	}
	void sigterm_handler(int signum) {
		state = !state;
	}
	void sigchld_handler(int signum, siginfo_t *si, void *data) {//it is needed
		(void)data;//whatever data has, make it void
		if(state) {
					// '->' is used to access a member of a struct which is referenced by the pointer in question
			signalID = (unsigned long)si->si_pid;//make it an unsigned long
			waitpid(-1, &status, WNOHANG | WUNTRACED); //We use WNOHANG so that we don't wait until a child stops, just checking if it did
			// -1 means that we expect a signal from any child
			// WUNTRACED returns if a child has stopped

			if (WIFSTOPPED(status)) { //If stopped then continue
				kill(signalID, SIGCONT);
			}

			else if ( signalID > 0) { 
				ignite = !ignite;
			}
		}	
	}
//Program entry point
int main(int argc, char* argv[]) {
	
	//checking for error
	if (argc != 2) {
		perror("We need 2 arguments");
		exit (EXIT_FAILURE);
	}
	
	char *str = argv[1];
	int n = strlen(str);
	int i;
	int k[n];
	
	//Checking if we have the right data typed
	for (i = 0; i < n; i++) {
		if (str[i] != 't' && str[i] != 'f') {
			perror("We need either t's or f's\n");
			exit (EXIT_FAILURE);
		}
	}
	
	for (i = 0; i < n; i++) {
		pid = fork();
		
		//paret process
		if (pid > 0) {
			k[i] = pid;
			printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n",getpid(), i, pid, str[i]);
		}
		//child process
		if (pid == 0) {
			char iternum[10];//argument has to be a string, but i is an integer
			sprintf(iternum, "%d", i);
			char * const args[] = {"./child", str, iternum, NULL};//these will be the arguments of child
			int created = execv("./child", args);
			//Checking for error
			if (created < 0) {
				perror("execv!\n"); 
				exit (EXIT_FAILURE);
			}
			
		}
		//Checking for error
		else if (pid<0) {
			perror("fork");
			exit (EXIT_FAILURE);
		}
	}
	struct sigaction sa, sa2, sa3;
	sa.sa_handler = sigusr1_handler;
	sa2.sa_handler = sigterm_handler;
	sa3.sa_sigaction = sigchld_handler;
	sa3.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGTERM, &sa2, NULL);
	sigaction(SIGCHLD, &sa3, NULL);
	
	int o;
	while(state) {

		if(sigusr1) {
			for (o = 0; o < n; o++) {		 
				int m = kill(k[o], SIGUSR1);
				//Cheking for error
				if (m<0) {
					perror("Can't send SIGUSR1\n");
					exit (EXIT_FAILURE);			
				}
			}
			sigusr1 = !sigusr1;
		}
		if (ignite) {
		i = -1;
		do {
			i++;
		}
		while(signalID != k[i]);//Checking which child died	 
		
		printf("[PARENT/PID=%d] Child %d with PID = %d exited!\n", getpid(), i, k[i]);
		newChild = !newChild;//Create a new child
		ignite = !ignite;
		}
		if (newChild) {
			
			int childID = fork();
			//Cheking for error
			if (childID < 0) {
				perror("newChild's fork\n");
			    exit(EXIT_FAILURE);
			}
			else if (childID == 0) {
				char iternum[10];// iternum = iteration number = i
				sprintf(iternum, "%d", i);//making i a string so we can pass it to the child
				char * const args[] = {"./child", str, iternum, NULL};
				int created = execv("./child", args);
				//Checking for error
				if (created < 0) {
					perror("execv!\n"); 
					exit (EXIT_FAILURE);
				}
			}
			else if (childID > 0) {//parent 
				k[i] = childID;//save new child's ID in the same position of the array that the child died
				printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n",getpid(), i, childID, str[i]);
			}
			newChild = !newChild;
		} 		
	}
	
	int j;
	for (j = 0; j < n; j++) { 
		printf("[PARENT/PID=%d] Waiting for %d children to exit\n", getpid(), n - j);	
		int a = kill(k[j], SIGTERM);
		//Checking for error
		if (a<0) {
			perror("Can't kill\n");
			exit (EXIT_FAILURE);
		}
		waitpid(k[j], &status, 0);
		printf("[PARENT/PID=%d] Child with PID=%d terminated successfully with exit status code %d!\n", getpid(), k[j], WEXITSTATUS(status));
		
	}
	
	printf("[PARENT/PID=%d] All children exited, terminating as well.\n", getpid());

	return 0;
}
