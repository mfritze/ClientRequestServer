#include "server_f.h"
#define MAXSIZE 4096
#define BACKLOG 200


int main(int argc, char ** argv){
	char * rbuffer, * wbuffer;
	int port, lfd, written, r, logFD; 
	pid_t pid;
	socklen_t clientlen;
	struct sockaddr_in s_addr, client;
	/*If debugging */
	/*
	int debugFD;
	*/

	if (argc != 4){
		fprintf(stderr, "Wrong number of args\n");
		exit(-1);
	}

	port = checkArgs(argv);
	logFD = open(argv[3], O_WRONLY);
	/*debugFD = open("DEBUGLOG", O_WRONLY); */

	if(logFD < 0){
		fprintf(stderr, "Error opening file: %s\n",argv[3] );
		exit(-1);
	}

	//daemon(1,1);

	memset(&s_addr, 0, sizeof(s_addr));

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = socket(AF_INET, SOCK_STREAM, 0); 

    if (lfd == -1)
    	err(1, "socket failed");

	if (bind(lfd, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1){
		err(1, "bind failed");
	}


	if (listen(lfd,BACKLOG) == -1){
		exit(-1);
	}

	while (1){
		int cfd;
		clientlen = sizeof(&client);
		cfd = accept(lfd, (struct sockaddr *)&client, &clientlen); 

		if (cfd == -1){
			exit(-1);
		}

		pid = fork();

		if (pid == -1){
			exit(-1);
		}

		if(pid == 0) {

			rbuffer = malloc(MAXSIZE);
			while((r = read(cfd, rbuffer, MAXSIZE)) == MAXSIZE){
				rbuffer = realloc(rbuffer, strlen(rbuffer)*2);
			}
			if(r < 0){
				exit(-1);
			}
			
			/*write(debugFD, "Readbuffer:\n", strlen(rbuffer));
			write(debugFD, rbuffer, strlen(rbuffer)); */

			wbuffer = handleRequest(rbuffer, argv[2]);

			written = write(cfd, wbuffer, strlen(wbuffer));
			if(written == -1){
				exit(-1);
			}

			flock(logFD, LOCK_EX);
			logEvent(logFD, rbuffer, wbuffer, written, strlen(wbuffer));
			flock(logFD, LOCK_UN);

			free(rbuffer);
			free(wbuffer); 
			exit(0);
		}
        close(cfd);
	}

    return 0;
}
