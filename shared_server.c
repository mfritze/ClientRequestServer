/* Matthew Fritze
 * 1360555
 * CMPUT 379
 * Assignment 2 */

#include "shared_server.h"
#define DATELEN 32
#define ENTRYLEN 256

void daemonize(){
    //TODO remove all fprintf messages
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	/*umask()  sets  the calling process's file mode creation mask (umask) to
       mask & 0777 (i.e., only the file permission bits of mask are used), and
       returns the previous value of the mask. */
    umask(0);
    
    /* Get maximum number of file descriptors */
    if(getrlimit(RLIMIT_NOFILE, &rl) < 0){
    	fprintf(stderr, "Can't get file limit\n");
    }

    /* Become a session leader to lose controlling terminal */
    if((pid = fork()) < 0){
    	fprintf(stderr, "Error forking\n");
    }else if (pid > 0){
    	exit(0);
    }
    setsid();

    /* Ensure future opens won't allocate controlling terminals */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) < 0){
    	fprintf(stderr, "Can't ignore SIGHUP\n");
    }
    if((pid = fork()) < 0){
    	  fprintf(stderr, "Can't fork\n");
    }
    if(pid > 0){
    	exit(0);
    }

	fclose(stdout);
    fclose(stderr);
    fclose(stdin);

}
int checkArgs(char ** argv){
	int port;
    DIR * serve_dir,* log_dir;
    DIR *opendir(const char *name);

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

    return port;
}

void logEvent(FILE * logFile, char * request, char * responce){
    char * tab = "\t", date[DATELEN], * host = "127.0.0.1", entry[ENTRYLEN];
    time_t rawT;
    struct tm * localT;

    time(&rawT);
    localT = localtime(& rawT);

    strftime(date, DATELEN, "%a %d %b %Y %T %Z", localT);

    //printf("Date: %s\n", date);
    strcat(entry, date);
    strcat(entry, tab);
    strcat(entry, host);
    strcat(entry, tab);
    strcat(entry, request);
    strcat(entry, tab);
    strcat(entry, responce);
    strcat(entry, "\n");
    fwrite(entry,1, strlen(entry), logFile);
}