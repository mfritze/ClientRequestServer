/* Matthew Fritze
 * 1360555
 * CMPUT 379
 * Assignment 2 */

#include "shared_server.h"
#define DATELEN 32
#define ENTRYLEN 256
#define MINLEN 16
#define OK 200
#define BAD 400
#define FORBIDDEN 403
#define NOTFOUND 404
#define SERVERERR 500

void daemonize(){
    //TODO remove all fprintf messages
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

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
    // TODO
    // serve_dir = opendir(argv[2]);
    // if(serve_dir == NULL){
    //     fprintf(stderr, "Directory:%s DNE\n", argv[2] );
    //     exit(-1);
    // }

    // log_dir = opendir(argv[3]);
    // if(log_dir == NULL){
    //     fprintf(stderr, "Directory:%s DNE\n", argv[3] );
    //     exit(-1);
    // }

    return port;
}

void logEvent(FILE * logFile, char * request, char * responce){
    char * tab = "\t",* date, * host = "127.0.0.1", entry[ENTRYLEN];

    date = malloc(DATELEN * sizeof(char));

    getDate(date);

    strcat(entry, date);
    strcat(entry, tab);
    strcat(entry, host);
    strcat(entry, tab);
    strcat(entry, request);
    strcat(entry, tab);
    strcat(entry, responce);
    strcat(entry, "\n");
    fwrite(entry,1, strlen(entry), logFile);

    free(date);
}

void getDate(char * date){
    struct tm * localT;
    time_t rawT;

    time(&rawT);
    localT = localtime(& rawT);

    strftime(date, DATELEN, "%a %d %b %Y %T %Z", localT);
    //return date;
}

char * handleRequest(char * request, char * serverPath){
    char * content = "Content-Type: text/html\n", 
         * contentLen = "Content-Length:",
         * ok = "HTTP/1.1 200 OK\n", 
         * bad = "HTTP/1.1 400 Bad Request\n", 
         * notFound = "HTTP/1.1 404 Not Found\n",
         * forbidden = "HTTP/1.1 403 Forbidden\n",
         * server_err = "HTTP/1.1 500 Internal Server Error\n",
         * rMessage = "TEMP";
    int valid; // (200: okay, 400: bad, 404: FNF, 403 Forbidden; 500: Server err)

    valid = isValid(request, serverPath);
    printf("Valid: %d\n", valid);


    return rMessage;
}

int isValid(char * request, char * serverPath){
    char * get = "GET",* http = "HTTP/1.1", c, getBuff[3], httpBuff[8], * serveAddr;
    int spaceCount = 0, i, start, end, dif, reqLen;

    reqLen = strlen(request);
    if(reqLen < MINLEN){
        return BAD;
    }

    for(i = 0; i < reqLen; i++){
        c = request[i];
        printf("C: %c\n",c );
        if(c == ' '){
            spaceCount++;
            if(spaceCount == 1){
                start = (i + 1);
            }
            else if(spaceCount == 2){
                end = i;
            }
        }
    }

    memcpy(getBuff, request, 3);

    if(strcmp(get, getBuff) != 0){
        printf("Bad request\n");
        return BAD; //Not a get request
    }

    memcpy(httpBuff, request + (end + 1), 8);
    if(strcmp(http, httpBuff) != 0){
        printf("Not http 1.1\n");
        return BAD;
    }
    //printf("HTTP buff: %s\n",httpBuff);

    dif = end - start;
    serveAddr = malloc(dif + 1);
    memcpy(serveAddr, request  + start, dif);
    serveAddr[dif] = '\0';
    //check validity of the file
    //printf("Addr:--%s--\n", serveAddr);

    serveAddr = realloc(serveAddr, (dif + 1) + strlen(serverPath));
    strcat(serverPath, serveAddr); //TODO remove the possible double //
    printf("ServeAddr: %s\n", serveAddr );
    if(access(serveAddr, F_OK) == -1){
        fprintf(stderr, "File:%s DNE\n", serveAddr);
        return NOTFOUND;
    }
    if(access(serveAddr, F_)){ //Permission
        return FORBIDDEN;
    }
    free(serveAddr);

    //test end newline
    // \n, \n\r\n, \n\n
    if( (request[reqLen - 1] != '\n') || 
       ((request[reqLen - 2] != '\r') && (request[reqLen - 3] != '\n')) ||
       ((request[reqLen - 1] != '\n') && (request[reqLen - 2] != '\n'))){
        printf("Doesnt end in a newline \n");
        return -1;
    }
    return 0;
}