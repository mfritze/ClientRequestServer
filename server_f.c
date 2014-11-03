#include "server_f.h"
#define MAXSIZE 4096
#define BACKLOG 200

static void kidhandler(int signum) {
	/* signal handler for SIGCHLD */
	waitpid(WAIT_ANY, NULL, WNOHANG);
}

int main(int argc, char ** argv){
	char rbuffer[MAXSIZE], * wbuffer;
	int port, lfd, w, written, r;
	pid_t pid;
	struct sockaddr_in s_addr, client;
	struct sigaction sa;
	socklen_t clientlen;
	FILE * logFile;

	if (argc != 4){
		fprintf(stderr, "Wrong number of args\n");
		exit(-1);
	}

	port = checkArgs(argv);
	logFile = fopen(argv[3], "w");
	logEvent(logFile, "Test 1", "Test 3");

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

	if (listen(lfd,BACKLOG) == -1)// TODO Change 3 to infinite
		err(1, "listen failed");
	/*
	 * sigemptyset() initializes the signal set given by set  to  empty,  
	 * with all signals excluded from the set.
     */
	sa.sa_handler = kidhandler;
    sigemptyset(&sa.sa_mask);
	/*
	 * we want to allow system calls like accept to be restarted if they
	 * get interrupted by a SIGCHLD
	 * The  sigaction()  system  call  is used to change the action taken by a
     *  process on receipt of a specific signal.
	 */
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) 
            err(1, "sigaction failed");
  
    /* Change the current working directory to the root
     so we won't prevent file systems from being unmounted */

        // This can be used to serve clients
  //    if(chdir(argv[2]) < 0){ 
  //   	fprintf(stderr, "Can't change directory to the root\n");
	 // }

	//fprintf(stderr,"Server up and listening for connections on port %u\n", port);


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
		 /* write the message to the client, being sure to
			 * handle a short write, or being interrupted b
			 * a signal before we could write anything.
			 */

			// w = 0;
			// written = 0;
			// while (written < strlen(buffer)) {
			// 	w = write(cfd, buffer + written, strlen(buffer) - written);
			// 	if (w == -1) {
			// 		if (errno != EINTR)
			// 			err(1, "write failed");
			// 	}
			// 	else
			// 		written += w;
			// }
			r = read(cfd, rbuffer, MAXSIZE);
			if(r < 0){
				err(1, "Read error \n");
			}
			//fprintf(stderr, "Message Read: %s\r\n", rbuffer);

			wbuffer = handleRequest(rbuffer, argv[2]);

			w = write(cfd, "test/n", 5);
			if(w == -1){
				err(1, "write failed");
			}
			exit(0);
		}
		//logEvent(logFile, "DOne", "Beep");
        close(cfd);
	}
    return 0;
}
