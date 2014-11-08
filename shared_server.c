/* Matthew Fritze
 * 1360555
 * CMPUT 379
 * Assignment 2 */

#include "shared_server.h"
#define DATELEN 32
#define ENTRYLEN 256
#define MINLEN 16
#define RESPONSELEN 26
#define OK 200
#define BAD 400
#define FORBIDDEN 403
#define NOTFOUND 404
#define SERVERERR 500


void daemonize(){
    /* The daemonize function I wrote (referencing the book)
    but never actually called */
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

    umask(0);
    
    /* Get maximum number of file descriptors */
    if(getrlimit(RLIMIT_NOFILE, &rl) < 0){
    	fprintf(stderr, "Can't get file limit\n");
        exit(-1);
    }

    /* Become a session leader to lose controlling terminal */
    if((pid = fork()) < 0){
    	fprintf(stderr, "Error forking\n");
        exit(-1);
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
        exit(-1);
    }
    if((pid = fork()) < 0){
    	  fprintf(stderr, "Can't fork\n");
          exit(-1);
    }
    if(pid > 0){
    	exit(0);
    }

	fclose(stdout);
    fclose(stderr);
    fclose(stdin);
}

int checkArgs(char ** argv){
    /*Checks that the port is a viable number,
    and that the directory from which the server is 
    serving documents is valid 
    In retuns the port number if no issues exist.*/
	int port;
    DIR * serve_dir;

	port = atoi(argv[1]);
    if((port > MAXPORT) || (port < 0)){
        fprintf(stderr, "Port: %d, does not exist\n", port);
        exit(-1);
    }

    serve_dir = opendir(argv[2]); /* argv2 serving from dir */
    if(serve_dir == NULL){
        fprintf(stderr, "Directory:%s Does not exist, exiting\n", argv[2]);
        exit(-1);
    }

    return port;
}

void logEvent(int logFile, char * request, char * response, 
int written, int total){
    /* Logs the server events into the logfile specified by 
    the user, to the format specified in the spec */
    //TODO make this work lol
    char * date, entry[ENTRYLEN] ,* indx, * head1, * head2, *temp;
    date = malloc(DATELEN * sizeof(char));
    getDate(date);
    indx = strstr(response, "OK");

    head1 = malloc(strlen(request));
    head2 = malloc(strlen(response));
    head1 = getHeader(request, head1);
    head2 = getHeader(response, head2);

    temp = strstr(head2, "4");
    if(!temp){
        temp = strstr(head2, "5");
    }
    if(!temp){
        temp = strstr(head2, "2");
    }
    //printf("head2:%s\n--temp:%s\n", head2, temp);
    head2 = temp;

    if(!indx) {
        snprintf(entry, ENTRYLEN, "%s\t127.0.0.1\t%s\t%s\n", 
            date, head1, head2);
    } else{
        snprintf(entry, ENTRYLEN, "%s\t127.0.0.1\t%s\t%s\t%d/%d\n", 
            date, head1, head2, written, total);        
    }

    write(logFile, entry, strlen(entry));
    free(date);
}

void getDate(char * date){
    /*Returns the date with the appropriate formatting,
    for any given timezone */
    struct tm * localT;
    time_t rawT;

    time(&rawT);
    localT = localtime(& rawT);

    strftime(date, DATELEN, "%a %d %b %Y %T %Z", localT);
}

char * handleRequest(char * request, char * serverPath){
    /* Given the request sent to the port, find out if it's 
    a valid get request or what is wrong with it.
    If it is valid, serve the requested document by returning it
    If invalid, return an HTTP response with the appropriate error
    message */
    char response[RESPONSELEN], * htmlresponse, * rMessage, * date, * fPath;
    int responseSize, fileSize, valid; 
    valid = isValid(request, serverPath); 

    switch(valid){
        case(OK):
            strcpy(response, "200 OK" );
            fPath = malloc(sizeof(char));
            getFileAddr(fPath, serverPath, request);
            htmlresponse = getResponse(fPath);
            break;
        case(BAD):
            strcpy(response, "400 Bad Request");
            htmlresponse = getResponse("400.html");
            break;
        case(FORBIDDEN):
            strcpy(response, "403 Forbidden");
            htmlresponse = getResponse("403.html");
            break;
        case(NOTFOUND):
            strcpy(response, "404 Not Found");
            htmlresponse = getResponse("404.html");
            break;
        case(SERVERERR):
            strcpy(response, "500 Internal Server Error");
            htmlresponse = getResponse("500.html");
            break;
    }

    date = malloc(sizeof(char)*DATELEN);
    getDate(date);

    fileSize = strlen(htmlresponse) - 1;
    /*64 is for the labels*/
    responseSize = fileSize + DATELEN + RESPONSELEN + 64; 
    rMessage = malloc(responseSize * sizeof(char));

    snprintf(rMessage, responseSize, 
    "HTTP/1.1 %s\n%s\nContent-Type: text/html\nContent-Length: %d\r\n\r\n%s",
    response, date, fileSize, htmlresponse);

    free(htmlresponse);
    free(date);
    return rMessage;
}

void getFileAddr(char * fPath, char * sPath, char * request){
    /*Given the request and the path to the servers documents,
    parse and return the path to the file requested 
    and store it in fPath. It is assumed the directory exists,
    since it was checked on startup */
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

    if((sPath[pathLen - 1] == '.')){
        sPath[pathLen - 1] = '\0';
        pathLen -= 1;
    }

    if((request[start] == '/') && (pathLen == 0)){
        start++;
    }

    dif = end - start;
    fPath = realloc(fPath, (pathLen + dif + 1) * sizeof(char)); 
    memcpy(fPath , sPath, pathLen);
    memcpy(fPath + pathLen, request  + start, dif); /*requested file*/
    fPath[pathLen + dif] = '\0';
}

int isValid(char * request, char * serverPath){
    /*Checks the request for formatting errors,
    such as no GET or HTTP/1.1, no trailing newline,
    or file errors. It returns the appropriate HTTP
    error, or 200 on success. */
    char   c,* getBuff, httpBuff[8], * serveAddr, *firsts,
    * get = "GET", * http = "HTTP/1.1", * seconds;
    int i, reqLen, start, end;

    FILE * testOpen;

    serveAddr = malloc(sizeof(char));

    reqLen = strlen(request);
    if(reqLen < MINLEN){
        return BAD;
    }

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

    getBuff = malloc(start - 1);
    firsts = strchr(request, ' ');
    //printf("firsts:%s, delta pointer: %d\n", firsts, firsts - request);

    if(!firsts){
        return BAD;
    }
    if((firsts - request) != 3){
        return BAD;
    }

    memcpy(getBuff, request, 3);

    if(strncmp(get, getBuff,3)){
        free(getBuff);
        return BAD; 
    }

    free(getBuff);

    seconds = strchr(request + 4, ' ');
    if(!seconds){
        return BAD;
    }

    memcpy(httpBuff, seconds + 1, 8);
    printf("httpBuff:%s\n", httpBuff);
    if(strncmp(http, httpBuff, 8) != 0){
        printf("bad http\n");
        return BAD;
    }

    /* Test trailing new line */
    printf("Request:%s\n", request);
    if(((request[reqLen - 1] != '\n') || (request[reqLen - 2] != '\n')) &&
       ((request[reqLen - 1] != '\n') || (request[reqLen - 2] != '\r') ||
        (request[reqLen - 3] != '\n'))){
        printf("bad newline\n");
        return BAD;
    }

    getFileAddr(serveAddr, serverPath, request);

    if((serveAddr == NULL)){
        printf("bad file adr\n");
        return BAD;
    }

    printf("Server addr: %s\n", serveAddr );
    if((testOpen = fopen(serveAddr, "r"))){ 
        fclose(testOpen);
    }
    else if(errno == ENOENT){
        return NOTFOUND; /* 404 */
    }else if(errno == EACCES){
        return FORBIDDEN; /* 403 */
    }else {
        return SERVERERR; /* 500 */
    }

    free(serveAddr); 

    return OK;
}

char * getResponse(char * addr){
    /* Makes the response to serve
    to the client */
    FILE * fp;
    char * response;
    int fileSize, r, read = 0;

    fp = fopen(addr, "r");

    if(fp == NULL){
        err(1, "Errer opening response file");
    }

    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    response = malloc(fileSize * sizeof(char));

    while(read < fileSize){
        r = fread(response, 1, fileSize, fp);
        if(r == 0){
            err(1, "Read failed");
        }
        read += r;
    }

    fclose(fp);
    return response;
}

char * getHeader(char * buffer, char * copy){
    /*Returns the first line of 
    the buffer */
    char * nL;
    memcpy(copy, buffer, strlen(buffer));

    nL = strchr(copy, '\n');
    if(!nL){
        return copy;
    }

    *nL = '\0';
    nL = strchr(copy, '\r');
    if(!nL){
        return copy;
    }
    *nL = '\0';
    return copy;
}