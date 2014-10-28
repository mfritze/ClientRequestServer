#include "server_f.h"
#define MAXSIZE 4096

int main(int argc, char ** argv){
	char message[MAXSIZE], buffer[MAXSIZE];
	int port, lfd, cfd;
	struct sockaddr_in s_addr;

	//fprintf(stderr,"Before daemon pid: %d\n", getpid());
	if(argc < 4){
		fprintf(stderr, "Not enough args\n");
		exit(-1);
	}

	port = checkArgs(argv);

	daemonize(message);

	lfd = socket(AF_LOCAL, SOCK_STREAM, 0);

	memset(&s_addr, '0', sizeof(s_addr));
    memset(buffer, '0', sizeof(buffer)); 

    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_port = htons(port); 

    bind(lfd, (struct sockaddr*)&s_addr, sizeof(s_addr)); 
	listen(lfd, 10000); // TODO Change this to infinite
	while(1){
		cfd = accept(lfd, (struct sockaddr*) NULL, NULL); 

        write(cfd, buffer, strlen(buffer)); 

        close(cfd);
	}
    return 0;
}