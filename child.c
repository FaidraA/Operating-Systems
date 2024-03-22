#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

time_t start;
bool state, sigusr1 = 0;
int i;

	void sig_handler(int signum){ 

		//sigalrm handler
		if (signum== SIGALRM) {
			if (state==true) {
				printf("[ID=%d/PID=%d/TIME=%ld] The gates are open!\n", i, getpid(),-(start - time(NULL)) ); 
				alarm (15);
			}
			else if (state==false) {
				printf("[ID=%d/PID=%d/TIME=%ld] The gates are closed!\n", i, getpid(),-(start - time(NULL)) );
				alarm(15);
			}
		} 
		//sigusr1 handler
		else if (signum == SIGUSR1) {
			sigusr1 = 1;
		} 
	
		//sigusr2 handler
		else if (signum == SIGUSR2) {
			state = !state; 
			if (state == true){
				printf("[ID=%d/PID=%d/TIME=%ld] The gates are open!\n", i, getpid(),-(start - time(NULL)) ); 
			}
			else if (state == false) {
				printf("[ID=%d/PID=%d/TIME=%ld] The gates are closed!\n", i, getpid(),-(start - time(NULL)) ); 
			}	
		}	
	}	
int main(int argc, char** argv) {

	start = time(NULL);
	
	char *str = argv[1];
	i = atoi(argv[2]);//argv[2] is a string but we want to save it as an int
		
	if (str[i] == 't'){
	state = 1;
	printf("[ID=%d/PID=%d/TIME=%ld] The gates are open!\n", i, getpid(),-(start - time(NULL)) );
	}
	else if (str[i] == 'f'){
	state = 0;
	printf("[ID=%d/PID=%d/TIME=%ld] The gates are closed!\n", i, getpid(),-(start - time(NULL)) );
	}
	
	struct sigaction sa;
	sa.sa_handler = &sig_handler;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	alarm(15);
	
	while (1) {
	
	
		if (sigusr1) {
			if (state){
				printf("[ID=%d/PID=%d/TIME=%ld] The gates are open!\n", i, getpid(),-(start - time(NULL)) ); 
			}
			else if (!state){
				printf("[ID=%d/PID=%d/TIME=%ld] The gates are closed!\n", i, getpid(),-(start - time(NULL)) );
			}
			sigusr1 = !sigusr1;
		}	
	}
	return 0;
}
