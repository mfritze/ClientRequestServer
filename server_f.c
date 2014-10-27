#include "server_f.h"
#define MAXSIZE 4096

int main(int argc, char ** argv){
	char message[MAXSIZE];
	int port;
	DIR * serve_dir,* log_dir;
    DIR *opendir(const char *name);


	//fprintf(stderr,"Before daemon pid: %d\n", getpid());
	if(argc < 4){
		fprintf(stderr, "Not enough args\n");
		exit(-1);
	}

	port = atoi(argv[1]);
	if((port > MAXPORT) || (port < 0)){
		fprintf(stderr, "Port: %d, does not exist\n", port);
		exit(-1);
	}

	serve_dir = opendir(argv[2]);
	if(serve_dir == NULL){
		fprintf(stderr, "Directory:%s DNE\n", argv[2] );
		exit(-1);
	}

	log_dir = opendir(argv[3]);
	if(log_dir == NULL){
		fprintf(stderr, "Directory:%s DNE\n", argv[3] );
		exit(-1);
	}

	daemonize(message);

	while(1);
    return 0;
}