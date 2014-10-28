#include "server_f.h"
#define MAXSIZE 4096

int main(int argc, char ** argv){
	char message[MAXSIZE], buffer[MAXSIZE];
	int port, lfd, cfd, w, written;
	pid_t pid;
	struct sockaddr_in s_addr, client;
	struct sigaction sa;
	socklen_t clientlen;

	//fprintf(stderr,"Before daemon pid: %d\n", getpid());
	if (argc < 4){
		fprintf(stderr, "Not enough args\n");
		exit(-1);
	}

	port = checkArgs(argv);

	daemonize(message);

	

	memset(&s_addr, 0, sizeof(s_addr));
    //memset(buffer, '0', sizeof(buffer)); 

    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_port = htons(port);

    lfd = socket(AF_INET, SOCK_STREAM, 0); 

    if (lfd == -1)
    	err(1, "socket failed");

	if (bind(lfd, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1)
		err(1, "bind failed");

	if (listen(lfd,3) == -1)// TODO Change this to infinite
		err(1, "listen failed");
	/*
	 * sigemptyset() initializes the signal set given by set  to  empty,  
	 * with all signals excluded from the set.
     */
	sa.sa_handler = waitpid(WAIT_ANY, NULL, WNOHANG);;
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


	while (1){
		clientlen = sizeof(&client);
		cfd = accept(lfd, (struct sockaddr*) & client, &clientlen); 
		
		if (cfd == -1)
		 	err(1, "accept failed");

		pid = fork();
		if (pid == -1)
		     err(1, "fork failed");

		if(pid == 0) {
		 /* write the message to the client, being sure to
			 * handle a short write, or being interrupted by
			 * a signal before we could write anything.
			 */

			w = 0;
			written = 0;
			while (written < strlen(buffer)) { //TODO this needs no change to writing to the proper location,
											//  and needs to log to the right location
				w = write(cfd, buffer + written, strlen(buffer) - written);
				if (w == -1) {
					if (errno != EINTR)
						err(1, "write failed");
				}
				else
					written += w;
			}
			exit(0);
		}
        //write(cfd, buffer, strlen(buffer)); 

        close(cfd);
	}
    return 0;
}