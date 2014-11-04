/* Matthew Fritze
 * 1360555
 * CMPUT 379
 * Assignment 2 */

#include "shared_server.h"
#define DATELEN 32
#define ENTRYLEN 256
#define MINLEN 16
#define RESPONCELEN 26
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
    DIR * serve_dir;
    DIR *opendir(const char *name);

	port = atoi(argv[1]);
    if((port > MAXPORT) || (port < 0)){
        fprintf(stderr, "Port: %d, does not exist\n", port);
        exit(-1);
    }
    // TODO
    serve_dir = opendir(argv[2]);
    if(serve_dir == NULL){
        fprintf(stderr, "Directory:%s DNE\n", argv[2] );
        exit(-1);
    }

    // log_dir = opendir(argv[3]);
    // if(log_dir == NULL){
    //     fprintf(stderr, "Directory:%s DNE\n", argv[3] );
    //     exit(-1);
    // }

    return port;
}

void logEvent(FILE * logFile, char * request, char * responce){
    char * date,  entry[ENTRYLEN];

    date = malloc(DATELEN * sizeof(char));
    getDate(date);

    snprintf(entry, ENTRYLEN, "%s\t127.0.0.1\t%s\t%s\n", date, request, responce);
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
    char responce[RESPONCELEN], * htmlResponce, * rMessage, * date;
    int responseSize, fileSize, valid; 

    valid = isValid(request, serverPath);
    printf("Valid: %d\n", valid);

    switch(valid){
        case(OK):
            strcpy(responce, "200 OK" );
            //htmlResponce = getResponce("")
            break;
        case(BAD):
            strcpy(responce, "400 Bad Request");
            htmlResponce = getResponce("400.html");
            break;
        case(FORBIDDEN):
            strcpy(responce, "403 Forbidden");
            htmlResponce = getResponce("403.html");
            break;
        case(NOTFOUND):
            strcpy(responce, "404 Not Found");
            htmlResponce = getResponce("404.html");
            break;
        case(SERVERERR):
            strcpy(responce, "500 Internal Server Error");
            htmlResponce = getResponce("500.html");
            break;
    }

    printf("responce: %s\n", responce );

    date = malloc(sizeof(char)*DATELEN);
    getDate(date);

    fileSize = strlen(htmlResponce);
    responseSize = fileSize + DATELEN + RESPONCELEN + 64; //64 is for the labels
    rMessage = malloc(responseSize * sizeof(char));

    snprintf(rMessage, responseSize, 
        "HTTP/1.1 %s\n%s\nContent-Type: text/html\nContent-Length: %d\n%s",
        responce, date, fileSize, htmlResponce);

    free(htmlResponce);
    free(date);
    return rMessage;

}


int isValid(char * request, char * serverPath){
    char * get = "GET",* http = "HTTP/1.1", c, getBuff[3], 
           httpBuff[8], * serveAddr;
    int spaceCount = 0, i, start, end, dif, reqLen, pathLen;
    FILE * testOpen;

    pathLen = strlen(serverPath);
    reqLen = strlen(request);
    if(reqLen < MINLEN){
        return BAD;
    }

    for(i = 0; i < reqLen; i++){
        c = request[i];
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

    if((serverPath[pathLen - 1] == '/') || (serverPath[pathLen - 1] == '.')){
        serverPath[pathLen - 1] = '\0';
        pathLen -= 1;

    }
    if((request[start] == '/') && (pathLen == 0)){
        start++;
    }

    dif = end - start;
    serveAddr = malloc(dif + 1);
    memcpy(serveAddr, request  + start, dif);
    serveAddr[dif] = '\0';
    //check validity of the file
    //printf("Addr:--%s--\n", serveAddr);

    serveAddr = realloc(serveAddr, (dif + 1) + pathLen);

    strcat(serverPath, serveAddr); //TODO remove the possible double //
    printf("serveAddr: %s\n", serveAddr );

    if((testOpen = fopen(serveAddr, "r"))){ //TODO this will be wrong if file exists but is forbidden
        fclose(testOpen);
    }else{
        fprintf(stderr, "File:%s can't be opened\n", serveAddr);
        return NOTFOUND;
    }

    if(access(serveAddr, F_OK) == -1){
        fprintf(stderr, "File:%s DNE\n", serveAddr);
        return FORBIDDEN;
    }

    free(serveAddr);

    //test end newline
    // \n, \n\r\n, \n\n
    if( (request[reqLen - 1] != '\n') || 
       ((request[reqLen - 2] != '\r') && (request[reqLen - 3] != '\n')) ||
       ((request[reqLen - 1] != '\n') && (request[reqLen - 2] != '\n'))){
        printf("Doesnt end in a newline \n");
        return BAD;
    }
    return OK;
}

char * getResponce(char * addr){
    FILE * fp;
    char * responce;
    int fileSize, rBytes;

    fp = fopen(addr, "r");

    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    printf("Size: fileSize %d\n", fileSize );

    responce = malloc(fileSize * sizeof(char));
    rBytes = fread(responce, 1, fileSize, fp);
    if(rBytes != fileSize){
        fprintf(stderr, "Errer Reading: %s, exiting \n", addr);
        exit(-1);
    }
    fclose(fp);

    return responce;
}