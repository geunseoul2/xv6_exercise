#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define DATASIZE (8*4096)

char data[DATASIZE];

int
main(int argc, char *argv[])
{
  if(argc != 2){
    printf("Usage: secret the-secret\n");
    exit(1);
  }

  strcpy(data, "This may help.");

  strcpy(data + 16, argv[1]);

  exit(0);
}

//After exiting, the secret process becomes a ZOMBIE state,
//and the parent, which is SH removes the resources of the child process
//this is when the freeproc function proceeds, which leads to kfree
//The secret memory is stored in freelist, which is LIFO structure

//Why make the freelist LIFO? -> since there is a L1/L2 cache in CPU
//it is faster to get the most recently used memory.

//so, to attack that point, we need to call kalloc in attack..?

