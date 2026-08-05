#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char buf[512];

int isDelimiter(char c) {
    const char delimiters[] = " -\r\t\n./,";
    for (int i = 0; delimiters[i] != '\0'; i++) {
        if (c == delimiters[i]) {
            return 1;
        }
    }
    return 0;
}

/*
For each input file, sixfive prints all the numbers in the file
that are multiples of 5 or 6.
got hints from grep, cat command function
*/
void sixfive(int fd) {
    int n;
    int isNum = 0, num = 0;

    while((n = read(fd, buf, sizeof(buf))) > 0) {
        //modify the buffer to only have multiples of 5 or 6
        char* ptr = buf;

        while(1) {
            char c = *ptr;
            if((c == '\0' || isDelimiter(c)) && isNum) { //if ch meet a delimiter or EOF
                if(num != 0) {
                    if(num % 5 == 0 || num % 6 == 0) printf("%d\n",num);
                    num = 0;
                    isNum = 0;
                }
            } else if(c >= '0' && c <= '9') {
                num = num*10 + (c - '0');
                isNum = 1;
            }
            if(c == '\0') break;

            ptr++;
        }
    }
    if(n < 0){
        fprintf(2, "sixfive: read error\n");
    }
}

int main(int argc, char *argv[]) {
    int fd,i;

    if(argc <= 1) {
        fprintf(2, "usage: print all the numbers that are multiples of 5 or 6");
        exit(1);
    }

    for(i = 1;i < argc; i++) {
        if((fd = open(argv[i],O_RDONLY)) < 0) {
            fprintf(2,"sixfive: cannot open %s\n",argv[i]);
            exit(1);
        }
        sixfive(fd);
        close(fd);
    }

    exit(0);
}