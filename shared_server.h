/* Matthew Fritze
 * 1360555
 * CMPUT 379
 * Assignment 2 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>


#define MAXPORT 65535

void daemonize();
int checkArgs(char **);
void logEvent(int , char * , char * , int ,int);
void getDate(char *);
char * handleRequest(char * , char *);
int isValid(char *, char *);
char * getResponse(char * );
void getFileAddr(char * fPath, char * sPath, char * request);
char * getHeader(char * buffer);
