/* Matthew Fritze
 * 1360555
 * CMPUT 379
 * Assignment 2 */
#define MAXSIZE 4096
#define BACKLOG 200

#include "server_p.h"

int main(int argc, char ** argv){
    int port, lfd, logFD, cfd; 
    socklen_t clientlen;
    struct sockaddr_in s_addr, client;

    struct pthread_params * params;

    pthread_t tid; 
    pthread_attr_t attr; 
    pthread_attr_init(&attr);
    /*If debugging */
    /*
    int debugFD;
    */

    if (argc != 4){
        fprintf(stderr, "Wrong number of args\n");
        exit(-1);
    }

    port = checkArgs(argv);
    logFD = open(argv[3], O_WRONLY);
    /*debugFD = open("DEBUGLOG", O_WRONLY); */

    if(logFD < 0){
        fprintf(stderr, "Error opening file: %s\n",argv[3] );
        exit(-1);
    }

    daemon(1,1);

    memset(&s_addr, 0, sizeof(s_addr));

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = socket(AF_INET, SOCK_STREAM, 0); 

    if (lfd == -1)
        err(1, "socket failed");

    if (bind(lfd, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1){
        err(1, "bind failed");
    }

    if (listen(lfd,BACKLOG) == -1){
        exit(-1);
    }


    while (1){
        clientlen = sizeof(&client);
        cfd = accept(lfd, (struct sockaddr *)&client, &clientlen); 

        if (cfd == -1){
            exit(-1);
        }
        params = malloc(sizeof(struct pthread_params));
        params->clientFD = cfd;
        params->serverDir = argv[2];
        params->logFD = logFD;
        pthread_create(&tid, &attr, tServe, (void *) params);

    }

    return 0;
}

void * tServe(void * params){
    char * rbuffer, * wbuffer;
    int r, written;
    struct pthread_params * p;

    p = (struct pthread_params *) params;

    rbuffer = malloc(MAXSIZE);
    while((r = read(p->clientFD, rbuffer, MAXSIZE)) == MAXSIZE){
        rbuffer = realloc(rbuffer, strlen(rbuffer)*2);
    }
    if(r < 0){
        exit(-1);
    }
    

    wbuffer = handleRequest(rbuffer, p->serverDir);

    written = write(p->clientFD, wbuffer, strlen(wbuffer));

    if(written == -1){
        exit(-1);
    }

    flock(p->logFD, LOCK_EX);
    logEvent(p->logFD, rbuffer, wbuffer, written, strlen(wbuffer));
    flock(p->logFD, LOCK_UN);

    free(rbuffer);
    free(wbuffer); 

    close(p->clientFD);
    //free(p);
    pthread_exit( (void *) 0 );
}