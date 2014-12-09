/* Matthew Fritze
 * 1360555
 * CMPUT 379
 * Assignment 2 */

#include "shared_server.h"
#include <pthread.h>


extern void daemonize(const char * );
extern char ** sortArgs(char ** );
void * tServe(void * );

struct pthread_params{
    int clientFD;
    int logFD;
    char * serverDir;

};
