/* Matthew Fritze
 * 1360555
 * CMPUT 379
 * Assignment 2 */

#include "shared_server.h"

void daemonize(const char * cmd){

	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	/*umask()  sets  the calling process's file mode creation mask (umask) to
       mask & 0777 (i.e., only the file permission bits of mask are used), and
       returns the previous value of the mask. */
    umask(0);

    /* Get maximum number of file descriptors */
    if(getrlimit(RLIMIT_NOFILE, &rl) < 0){
    	fprintf(stderr, "Can't get file limit: %s\n", cmd);
    }

    /* Become a session leader to lose controlling terminal */
    if((pid = fork()) < 0){
    	fprintf(stderr, "Error forking: %s\n", cmd);
    }else if (pid > 0){
    	exit(0);
    }
    setsid();

    /* Ensure future opens won't allocate controlling terminals */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) < 0){
    	fprintf(stderr, "Can't ignore SIGHUP %s\n", cmd);
    }
    if((pid = fork()) < 0){
    	  fprintf(stderr, "Can't fork %s\n", cmd);
    }
    if(pid > 0){
    	exit(0);
    }

    /* Change the current working directory to the root
     so we won't prevent file systems from being unmounted */
    if(chdir("/") < 0){ /* TODO is this supposed to be an arg? */
    	fprintf(stderr, "Can't change directory to the root %s\n", cmd);
	}

	/*Close all file descriptors */
	if(rl.rlim_max == RLIM_INFINITY){
		rl.rlim_max = 1024;
	}
	for(i = 0; i < rl.rlim_max; i++){
		close(i);
	}

	/* attach file descriptors 0,1,2 to /dev/null */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	/* Initialize the log file */
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if( fd0 != 0 || fd1 != 1 || fd2 != 2){
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d\n" ,fd0, fd1, fd2);
		exit(-1);
	}
}

void checkArgs(char ** argv){
	int port;
	
	port = atoi(argv[1]);
	if((port > MAXPORT) || (port < 0)){
		fprintf(stderr, "Port: %d, does not exist\n", port);
		exit(-1);
	}

}
