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
    //TODO remove all fprintf messages
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

void logEvent(FILE * logFile, char * request, char * response, int written, int total){
    //TODO make this work lol
    char * date, entry[ENTRYLEN] ,* indx, * head1, * head2;
    //printf("--Request--\n%s\n--Response--\n%s\n", request, response);
    date = malloc(DATELEN * sizeof(char));
    getDate(date);
    //printf("--Date--\n%s\n", date );
    indx = strstr(response, "OK");

    head1 = getHeader(request);
    head2 = getHeader(response);

    printf("--Headers--\n-H1:%s\n-H2:%s\nnewline", head1, head2);
    //printf("H1--%s--\n", head1);
    //printf("--indx--\n%s\n",indx );
    if(!indx) {
        snprintf(entry, ENTRYLEN, "%s\t127.0.0.1\t%s\t%s\n", 
            date, head1, head2);
    } else{
        snprintf(entry, ENTRYLEN, "%s\t127.0.0.1\t%s\t%s\t%d/%d\n", 
            date, head1, head2, written, total);        
    }
    //printf("--Pre write--\n--Entry--\n%s\n", entry);
    fwrite(entry,1, strlen(entry), logFile);
    //printf("Post write\n");
    free(head1);
    free(head2);
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
    responseSize = fileSize + DATELEN + RESPONSELEN + 64; //64 is for the labels
    rMessage = malloc(responseSize * sizeof(char));

    snprintf(rMessage, responseSize, 
        "HTTP/1.1 %s\n%s\nContent-Type: text/html\nContent-Length: %d\r\n\r\n%s",
        response, date, fileSize, htmlresponse);

    free(htmlresponse);
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
    char  * get = "GET",* http = "HTTP/1.1", c,* getBuff, 
            httpBuff[8], * serveAddr;
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
    memcpy(getBuff, request, start); //TODO this needs to check until the first space
    //printf("Get buff: %s\n",getBuff );

    if(strcmp(get, getBuff)){
        //printf("Bad request\n");
        free(getBuff);
        return BAD; 
    }

    free(getBuff);


    memcpy(httpBuff, request + (end + 1), 8);
    if(strcmp(http, httpBuff) != 0){
        //printf("Not http 1.1\n");
        return BAD;
    }

    getFileAddr(serveAddr, serverPath, request);


    //printf("The returned file address: %s\n", serveAddr);
    if((serveAddr == NULL)){
        //fprintf(stderr, "bad address%s\n", serveAddr);
        return BAD;
    }
    if((testOpen = fopen(serveAddr, "r"))){ //TODO this will be wrong if file exists but is forbidden
        fclose(testOpen);
    }
    else if(errno == ENOENT){
        //fprintf(stderr, "File:%s can't be opened\n", serveAddr);
        return NOTFOUND; /* 404 */
    }else if(errno == EACCES){
        //fprintf(stderr, "File:%s DNE\n", serveAddr);
        return FORBIDDEN; /* 403 */
    }else {
        //fprintf(stderr, "Server err\n");
        return SERVERERR; /* 500 */
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

char * getResponse(char * addr){
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

    //printf("Size: fileSize %d\n", fileSize );

    response = malloc(fileSize * sizeof(char));

    while(read < fileSize){
        r = fread(response, 1, fileSize, fp);
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

    return response;
}

char * getHeader(char * buffer){
    int i, c = 1;
    char * header;
    for(i = 1; i < strlen(buffer); i++){
        if((buffer[i - 1] == '\r')  && (buffer[i] == '\n')){
            c = i - 1;
            break;
        }
        if(buffer[i] == '\n'){
            c = i;
            break;
        }
    };


    header = malloc(c); // TODO make sure to free
    memcpy(header, buffer, c);

    return header;
}