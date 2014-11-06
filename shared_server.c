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

	port = atoi(argv[1]);
    if((port > MAXPORT) || (port < 0)){
        fprintf(stderr, "Port: %d, does not exist\n", port);
        exit(-1);
    }
    // TODO
    serve_dir = opendir(argv[2]); //argv2 is server files
    if(serve_dir == NULL){
        fprintf(stderr, "Directory:%s DNE\n", argv[2] );
        exit(-1);
    }

    //TODO try permission to write to this file
    // log_dir = opendir(argv[3]);
    // if(log_dir == NULL){
    //     fprintf(stderr, "Directory:%s DNE\n", argv[3] );
    //     exit(-1);
    // }

    return port;
}

void logEvent(FILE * logFile, char * request, char * responce, int written, int total){
    //TODO make this work lol
    char * date,  entry[ENTRYLEN],* indx;

    date = malloc(DATELEN * sizeof(char));
    getDate(date);

    indx = strstr(responce, "OK");

    if(indx == NULL){
        snprintf(entry, ENTRYLEN, "%s\t127.0.0.1\t%s\t%s\n", 
            date, request, responce);
    }else{
        snprintf(entry, ENTRYLEN, "%s\t127.0.0.1\t%s\t%s\t%d/%d\n", 
            date, request, responce, written, total);        
    }

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
    char responce[RESPONCELEN], * htmlResponce, * rMessage, * date, * fPath;
    int responseSize, fileSize, valid; 

    valid = isValid(request, serverPath); // should I just call getFileAddr first and pass it along?
    //printf("Valid: %d\n", valid);

    switch(valid){
        case(OK):
            strcpy(responce, "200 OK" );
            fPath = malloc(sizeof(char));
            getFileAddr(fPath, serverPath, request);
            htmlResponce = getResponce(fPath);
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

    //printf("responce: %s\n", responce );

    date = malloc(sizeof(char)*DATELEN);
    getDate(date);

    fileSize = strlen(htmlResponce) - 1;
    responseSize = fileSize + DATELEN + RESPONCELEN + 64; //64 is for the labels
    rMessage = malloc(responseSize * sizeof(char));

    snprintf(rMessage, responseSize, 
        "HTTP/1.1 %s\n%s\nContent-Type: text/html\nContent-Length: %d\r\n\r\n%s",
        responce, date, fileSize, htmlResponce);
    //printf("\nServer Responce:\n%s\n", rMessage);
    free(htmlResponce);
    free(date);
    //1free(fPath);
    return rMessage;
}

void getFileAddr(char * fPath, char * sPath, char * request){
    int start = 0, end , reqLen, pathLen, dif, i;
    char c;

    pathLen = strlen(sPath);
    reqLen = strlen(request);

    for(i = 0; i < reqLen; i++){
        c = request[i];
        if(c == ' '){
            if(start == 0){
                start = i + 2;
            }
            else{
                end = i;
                break;
            }
        }
    }
    //TODO make sure the leading char is a /
    //fprintf(stderr, "The provided address:%s\n", request + start );
    //if((sPath[pathLen - 1] == '/') || (sPath[pathLen - 1] == '.')){
    if((sPath[pathLen - 1] == '.')){
        sPath[pathLen - 1] = '\0';
        pathLen -= 1;
    }

    if((request[start] == '/') && (pathLen == 0)){
        start++;
    }

    dif = end - start;
    fPath = realloc(fPath, (pathLen + dif + 1) * sizeof(char)); // TODO remember to free
    memcpy(fPath , sPath, pathLen);
    memcpy(fPath + pathLen, request  + start, dif); //requested file
    //fPath[dif] = '\0';
    //fPath = realloc(fPath, (dif + 1) + pathLen);
    //strcat(fPath, sPath); //TODO remove the possible double //
    //printf("Full Path: %s\n", fPath ); 
}

int isValid(char * request, char * serverPath){
    char * get = "GET",* http = "HTTP/1.1", c,* getBuff, 
           httpBuff[8], * serveAddr;
    int i, reqLen, start, end;
    FILE * testOpen;

    // pathLen = strlen(serverPath);
    serveAddr = malloc(sizeof(char));

    reqLen = strlen(request);
    if(reqLen < MINLEN){
        return BAD;
    }
    printf("Hi there\n");
    for(i = 0; i < reqLen; i++){
        c = request[i];
        if(c == ' '){
            if(start == 0){
                start = i;
            }
            else{
                end = i;
                break;
            }
        }
    }
    printf("Start: %d\n",start );
    getBuff = malloc(start - 1);
    memcpy(getBuff, request, start); //TODO this needs to check until the first space
    printf("Get buff: %s\n",getBuff );
    if(strcmp(get, getBuff) != 0){
        printf("Bad request\n");
        free(getBuff);
        return BAD; //Not a get request
    }

    free(getBuff);

    memcpy(httpBuff, request + (end + 1), 8);
    if(strcmp(http, httpBuff) != 0){
        printf("Not http 1.1\n");
        return BAD;
    }


    getFileAddr(serveAddr, serverPath, request);

    //printf("The returned file address: %s\n", serveAddr);
    if(serveAddr == NULL){
        fprintf(stderr, "bad address%s\n", serveAddr);
        return BAD;
    }
    if((testOpen = fopen(serveAddr, "r"))){ //TODO this will be wrong if file exists but is forbidden
        fclose(testOpen);
    }
    else if(errno == ENOENT){
        fprintf(stderr, "File:%s can't be opened\n", serveAddr);
        return NOTFOUND;
    }else if(errno == EACCES){
        fprintf(stderr, "File:%s DNE\n", serveAddr);
        return FORBIDDEN;   
    }else {
        fprintf(stderr, "Server err\n");
        return SERVERERR;
    }


    //free(serveAddr); //TODO

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
    int fileSize, r, read = 0;

    fp = fopen(addr, "r");

    if(fp == NULL){
        err(1, "Errer opening responce file");
    }

    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    //printf("Size: fileSize %d\n", fileSize );

    responce = malloc(fileSize * sizeof(char));

    while(read < fileSize){
        r = fread(responce, 1, fileSize, fp);
        if(r == 0){
            err(1, "Read failed");
        }
        read += r;
    }

    // if(rBytes != fileSize){
    //     fprintf(stderr, "Errer Reading: %s, exiting \n", addr);
    //     exit(-1);
    // }
    fclose(fp);

    return responce;
}
