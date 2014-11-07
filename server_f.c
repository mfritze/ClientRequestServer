#include "server_f.h"
#define MAXSIZE 4096
#define BACKLOG 200

static void kidhandler(int signum) {
	/* signal handler for SIGCHLD */
	waitpid(WAIT_ANY, NULL, WNOHANG);
}

int main(int argc, char ** argv){
	char rbuffer[MAXSIZE], * wbuffer; //TODO make rbuffer dynamically allocated
	int port, lfd, w, written, r;
	pid_t pid;
	socklen_t clientlen;
	FILE * logFile;
	struct sockaddr_in s_addr, client;
	struct sigaction sa;
	struct flock logLock;

	if (argc != 4){
		fprintf(stderr, "Wrong number of args\n");
		exit(-1);
	}

	/*logLock.l_type = F_WRLCK;
	logLock.l_whence = SEEK_SET;
	logLock.l_start = 0;
	logLock.l_len = 0;
	logLock.l_pid = getpid(); */

	port = checkArgs(argv);
	logFile = fopen(argv[3], "w");

	//daemonize();

	memset(&s_addr, 0, sizeof(s_addr));

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = socket(AF_INET, SOCK_STREAM, 0); 

    if (lfd == -1)
    	err(1, "socket failed");

	if (bind(lfd, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1)
		err(1, "bind failed");

	if (listen(lfd,BACKLOG) == -1)
		err(1, "listen failed");

	sa.sa_handler = kidhandler;
    sigemptyset(&sa.sa_mask);
	
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
            err(1, "sigaction failed");


	while (1){
		int cfd;
		clientlen = sizeof(&client);
		cfd = accept(lfd, (struct sockaddr *)&client, &clientlen); 
		
		if (cfd == -1)
		 	err(1, "accept failed");

		pid = fork();
		if (pid == -1)
		     err(1, "fork failed");

		if(pid == 0) {
			r = read(cfd, rbuffer, MAXSIZE); 
			//TODO maybe same as write below
			if(r < 0){
				err(1, "Read error \n");
			}

			wbuffer = handleRequest(rbuffer, argv[2]);
			written = 0;

			while(written < strlen(wbuffer)){
				w = write(cfd, wbuffer + written, strlen(wbuffer) - written);
				if(w == -1){
					err(1, "write failed");
				}else{
					written += w;
				}
			}

			// this needs to be blocking
			//flockfile
			//funlockfile
			//logLock.l_pid = getpid();
			//fcntl()
			printf("About to log\n");
			logEvent(logFile, rbuffer, wbuffer, written, strlen(wbuffer));


			printf("Finished logging\n");
			free(wbuffer); // Is this right?
			exit(0);
		}
        close(cfd);
	}
    return 0;
}
