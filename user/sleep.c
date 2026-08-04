#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[]) {
    int i;

    if(argc < 2) {
        fprintf(2, "Error: forgot to pass an argument...\n");
        exit(0); //the value in exit represents the status of the process
                 // 1 : success 0 : error
    }

    for(i = 1; i < argc; i++) {
        int tick = atoi(argv[i]);

        if(tick >= 0) pause(tick); //pause syscall(in sysproc.c) -> acquires the ticklock and sleep(kernel function)
        else {
            fprintf(2,"Error: the tick has to be a positive value");
            exit(0);
        }
    }

    exit(0);
}