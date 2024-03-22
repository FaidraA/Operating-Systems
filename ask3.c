#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <limits.h>


//Program entry point
int main(int argc, char **argv)
{
	//checking for error
	if (argc != 3) {
		perror("We need 3 arguments!\n");
		exit (EXIT_FAILURE);
	}

	int status;
	int state = 1;
	pid_t pid;
	int i;
    int n = atoi(argv[1]);//argv is an array of strings, but atoi changes a string to an int
	int k = atoi(argv[2]);	
	
	//Checking for error
	if (k == 0 && n <= 0) {
		perror("Wrong arguments!\n");
		exit (EXIT_FAILURE);
	}
	
	if (k == 0) {
		printf("1\n");
		return 0;
	}
	
	//Checking for error
	if (k < 0) {
		perror("k has to be a non negative number!\n");
		exit (EXIT_FAILURE);
	}
	
	//Checking for error
	if (n <= 0) {
		perror("We don't have a child to calculate!\n");
		exit(EXIT_FAILURE);
	}
	
	int pd[2*n]; //We need n pipes, and every pipe needs 2 numbers, so we need 2n array size 
	int counter = 0;		
	unsigned long long ans;//to get the highest posible number (for k == 20)
	int end = 1;
	int times = 0;


//We have pd[0,1] for child n->1, pd[2,3] child 1->2 ... pd[2n-1, 2n] child n-1 -> n
	for (i = 0; i < n; i++) {
		if(pipe(pd + i*2) < 0) {
            perror("Couldn't pipe");
            exit(EXIT_FAILURE);
        }
	}	
	
	for (i = 0; i < n; i++) {
	
		counter += 2;
		pid = fork();
		
		if (pid < 0) {
			perror("fork!\n");
			exit (EXIT_FAILURE);
		}
		
		//Child process
		if (pid == 0) {
			
			if (i >= k) {	
				exit (0);
			} // for example ./ask3 10 6, we don't want child 7,8,9,10 to do anything(child 7 -> i = 6) 			
			while(state) { 
				
				read(pd[counter-2], &ans, sizeof(ans));
				if (n == 1) { //We only have 1 child so we need to send the ans to ourselves
					printf("child[%d] got ans = %lld\n", i, ans);//den to theloume ayto, alla kalo einai na vlepoume oti paei se kathe paidi
					times++;
					ans = ans*times;
					if (times == k) {//if we get the answer the first time that we get to a child
						printf("%lld\n", ans);
						state = 0;
					}
					write(pd[1], &ans, sizeof(ans));
				}
				else {
				close(pd[counter-1]);//we dont use this end of the pipe
				printf("child[%d] got ans = %lld\n", i, ans);//den to theloume ayto, alla kalo einai na vlepoume oti paei se kathe paidi
				times++;//how many times did this child read the ans
				if (times == 1) {
					ans = ans*(i+1);
					end = i+1;
					if (end == k) {
						printf("%lld\n", ans);
						state = 0;
					}
				}
				if (times > 1) {
					end = end + n;//if it came back here then n-1 children changed the ans, so increase end by n
					
					if (end == k) {
						ans = ans*end;
						printf("%lld\n", ans);
						state = 0;
					}
					ans = ans*end;
					
				}
				if (i == n-1) { //If this is the last child, write to the first one
					
					write(pd[1], &ans, sizeof(ans));
				}
				else {
					write(pd[counter + 1], &ans, sizeof(ans));
				}
				if (k - end < n || n == k) {//it won't come back to this child, if n==k it would return the answer 2 times
					state = 0;
				}
				}//else bracket
			}
			exit(0);
		}
		
		//Parent process
		if (pid > 0) {
			if (i == 0) {
				close(pd[0]);
				ans = 1;
				write(pd[1], &ans, sizeof(ans));//send ans to child 1
			}
		
		}	
	
	}
	int wpid;
	while ((wpid = wait(&status)) > 0); // this way, the father waits for all the child processes 
	
	for (i = 0; i<2*n; i++) {
		close(pd[i]);
	}
	

	return 0;
}
