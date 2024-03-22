#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>  // u_short
#include <sys/socket.h>
#include <sys/select.h>
#include <stddef.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h> //For errno - the error number

#define MAX(a, b) ((a) > (b) ? (a) : (b))

char *hostname = "lab4-server.dslab.os.grnetcloud.net";
char ip[100];

int hostname_to_ip(char * hostname , char* ip) {
	struct hostent *he;	
	struct in_addr **addr_list;
	int i;
		
	if ( (he = gethostbyname( hostname ) ) == NULL) 
	{
		// get the host info
		perror("gethostbyname");
		exit(EXIT_FAILURE);
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	
	for(i = 0; addr_list[i] != NULL; i++) 
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	
	return 1;
	
}

int main(int argc, char** argv)
{
	int sd;
	int PORTNUM = 18080;
	char argm[50];
	int p3;
	int deb=0;

	if(argc>1){	
		for (int i=1;i<argc;i++){
			strcpy(argm,argv[i]);
			if (strncmp(argm, "--host", 6)==0){     //this is HOST
		    		char *p1;
				p1=strchr(argm,' ');
				*hostname= *p1;
			}
			if (strncmp(argm, "--port", 6)==0){     //this is PORT
				char *p2;
				p2=strchr(argm, ' ');
				p3=atoi(p2);
				PORTNUM = p3;
			}
			if (strncmp(argm, "--debug", 7)==0){     //this is debug
				deb=1;
			}
		}
	}
	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket!\n");
		exit (EXIT_FAILURE);
	}

	hostname_to_ip(hostname , ip);
	
	
	//this is the binding process
	/*struct sockaddr_in sinb;
    sinb.sin_family = AF_INET;	//Internet domain
   	sinb.sin_port = htons(0);	//choose random available port
   	sinb.sin_addr.s_addr = htonl(ip);	//ability to connect to given ip

   	if (bind(sd,&sinb,sizeof(sinb)) == -1){
	herror("Bind!");
	exit(EXIT_FAILURE);
    }*/
    	
    	
	//inet_addr :Use of this function is problematic because -1 is a valid address (255.255.255.255)

	struct sockaddr_in sin;
	
	sin.sin_family = AF_INET; /* Internet */
	sin.sin_port = htons((u_short) PORTNUM);
	
	
	if (inet_pton(AF_INET, ip, &sin.sin_addr) <= 0) {
		perror("Inet!\n");
		exit (EXIT_FAILURE);
	}
	
	if (connect(sd, (struct sockaddr *)&sin , sizeof(sin)) < 0) {
		perror("Connect error!\n");
		exit (EXIT_FAILURE);
	}
	printf("Connected to server!\n");
	char str[50];
	//int n = 100;
	int count;
	int state = 1;
	char *message;
	char server_reply[80];


	while (state) {
		fd_set inset;
        int maxfd;

        FD_ZERO(&inset);                // we must initialize before each call to select
        FD_SET(STDIN_FILENO, &inset);   // select will check for input from stdin
        FD_SET(sd, &inset);          // select will check for input from pipe

        // select only considers file descriptors that are smaller than maxfd
        maxfd = MAX(STDIN_FILENO, sd) + 1;
	
		struct timeval selTimeout;   //this is how we impose the time limit
		selTimeout.tv_sec=60;	     //1 minute time-out
		selTimeout.tv_usec=0;
		
		// wait until any of the input file descriptors are ready to receive
        int ready_fds = select(maxfd, &inset, NULL, NULL, &selTimeout);
        if (ready_fds <= 0) {
            perror("Time out!");
            continue;                                       // just try again
        }
		
	
		if (read(STDIN_FILENO,str,sizeof(str)) < 0) {
			printf("read error");
			continue;
		}
		int som;
		//When 2 strings are equal at strcmp function then it returns 0

		//get command
		if (strcmp(str, "get\n") == 0) {
			//message = "get";
				if (write(sd, str, sizeof(str)) < 0) {
					printf("write failed");
					continue;
				}
				if (deb == 1) {
					printf("[DEBUG] sent %s",str);
				}
			
			//Receive a reply from the server
			//if (FD_ISSET(sd, &inset)) {
				if (read(sd, server_reply, sizeof(server_reply)) < 0) {
					printf("read failed");
					continue;
				}
				if (deb == 1) {
					printf("[DEBUG] read %s\n",server_reply);
				}
			//}
			char newString[10][10];
		    int j=0, ctr=0;
			for(int i=0;i<=(strlen(server_reply));i++) {
				// if space or NULL found, assign NULL into newString[ctr]
				if(server_reply[i]==' '||server_reply[i]=='\0') {
					newString[ctr][j]='\0';
					ctr++;  //for next word
					j=0;    //for next word, init index to 0
				}
				else {
					newString[ctr][j]=server_reply[i];
					j++;
				}
			}
			//Convert from unix timestamp to human time
			int val = atoi(newString[3]);
		    time_t rawtime = val;
			struct tm  ts;
			char       buf[80];
			// Format time, "yyyy-mm-dd hh:mm:ss"
			ts = *localtime(&rawtime);
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", &ts);
			
			//to get rid of trailling zeros we use %g instead of %f
			printf("Latest event:\nInterval (%s)\nTemperature is: %g\nLight level is: %d\nTimestamp is: %s\n",newString[0], atoi(newString[2])/100.0, atoi(newString[1]), buf);
			memset(str,0, sizeof(str)); //empty the str array for further use
			memset(server_reply,0, sizeof(server_reply)); //empty the server_reply array for further use
		}

		//help command
		if (strcmp(str, "help\n") == 0) {
			if (deb == 1) {
				printf("[DEBUG] sent %s\n",str);
			}
			printf("Type 'help' for a help message\n");
			printf("Type 'exit' to exit this program\n");
			printf("Type 'get' to get the server's state\n");
			printf("Type 'N name surname reason' to exit to quarantine\n");
			memset(str,0, sizeof(str)); //empty the str array for further use
		}

		//exit command
		if (strcmp(str, "exit\n") == 0) {
			printf("This program will terminate...\n");
			state = 0;
		}

		//N name surname reason command
		count = 1; //how many word we sent
		for (int i = 0; str[i] != '\0';i++) {
		    if (str[i] == ' ' && str[i+1] != ' ')
		        count++;
		}
	
		if (count == 4)	{//count = 3 equals to 4 words 
			if (write(sd, str, sizeof(str)) < 0) {
				printf("write failed");
				continue;
			}
			if (deb == 1) {
					printf("[DEBUG] sent %s",str);
			}
			memset(str,0, sizeof(str)); //empty the str array for further use
			
			//Receive a reply from the server
			if (read(sd, server_reply, sizeof(server_reply)) < 0) {
				printf("read failed");
				continue;
			}
			if (deb == 1) {
					printf("[DEBUG] read %s\n\n",server_reply);
			}
			if (strncmp(server_reply,"try again",9) != 0) { //if server sent try again then don't go to the start
				printf("Send verification code: %s\n",server_reply);
				memset(server_reply,0, sizeof(server_reply)); //empty the server_reply array for further use	
				if (read(STDIN_FILENO,str,sizeof(str)) < 0) { //type the verification code
					printf("read failed");
					continue;
				}
				if (write(sd, str, sizeof(str)) < 0) {   //send it back
					printf("write failed");
					continue;
				}
				if (deb == 1) {
						printf("[DEBUG] sent %s",str);
				}
				memset(str,0, sizeof(str)); //empty the str array for further use
				if (read(sd, server_reply, sizeof(server_reply)) < 0) {  //server's reply
					printf("read failed");
					continue;
				}
				if (deb == 1) {
					printf("[DEBUG] read %s\n",server_reply);
				}
				printf("Response: %s",server_reply);
				memset(server_reply,0, sizeof(server_reply)); //empty the server_reply array for further use
			}
		}
		memset(str,0, sizeof(str)); //empty the str array for further use
	}
	
	return 0;
}
