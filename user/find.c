#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

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
void find(char* path, char* target) {
    int fd;
    struct stat st;
    char buf[512], *p;
    struct dirent de;

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
            
            if(st.type == T_FILE && strcmp(fmtname(buf), target) == 0) printf("%s\n",buf);
            if(st.type == T_DIR ) {
                if(strcmp(fmtname(buf),"..") == 0 || strcmp(fmtname(buf),".") == 0) continue;
                find(buf,target);
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

    find(argv[1],argv[2]);
    exit(0);
}