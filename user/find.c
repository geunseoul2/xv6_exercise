#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  if(strlen(p) >= DIRSIZ)
    return p;

  memmove(buf, p, strlen(p));
  //removed the padding to DIRSIZ to use strcmp
  buf[strlen(p)] = '\0';
  return buf;
}

/* 
find all the files in a directory tree with a specific name
Ideas from ls.c
*/
void find(char* path, char* target, int is_exec, char** cmd, int exec_argc) {
    int fd;
    struct stat st;
    char buf[512], *p;
    struct dirent de;
    int pid;

    if((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    if(st.type == T_DIR) {
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
            return;
        }
        strcpy(buf,path);
        p = buf + strlen(buf);
        *p++ = '/';
        while(read(fd,&de,sizeof(de)) == sizeof(de)) {
            if(de.inum == 0) continue;
            memmove(p,de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            
            if(st.type == T_FILE && strcmp(fmtname(buf), target) == 0) {
                if(is_exec) {
                    pid = fork();
                    if(pid < 0) {
                        printf("find: fork failed\n");
                        exit(1);
                    }
                    if(pid == 0) { // child
                        // execute cmd with the found file path as argument
                        char* args[MAXARG];
                        for(int i=0; i < exec_argc; i++) args[i] = cmd[i];
                        args[exec_argc] = buf;
                        args[exec_argc+1] = 0;
                        exec(cmd[0], args);
                        printf("find: exec failed\n");
                        exit(1);
                    } else { // parent
                        wait(0);
                    }
                } else {
                    printf("%s\n",buf);
                }
            }
            if(st.type == T_DIR ) {
                if(strcmp(fmtname(buf),"..") == 0 || strcmp(fmtname(buf),".") == 0) continue;
                find(buf,target,is_exec,cmd,exec_argc);
            }

        }

    } else {
        fprintf(2, "find: wrong directory %s\n", path);
        return;
    }
    close(fd);
}


int main(int argc, char *argv[]) {

    if(argc < 2) {
        fprintf(2, "find: wrong input format\n");
        exit(0);
    }
    
    if(argc == 3) find(argv[1],argv[2],0,0,0);
    else if(strcmp(argv[3],"-exec") == 0) find(argv[1],argv[2],1,&argv[4],argc-4);
    exit(0);
}