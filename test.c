#include <stdio.h>
#include <string.h>
#include <stdlib.h>



void read_context(int tid) {
    char command[256];
    
    sprintf(command, "cat /proc/getpid()/task/%d/maps", tid);

