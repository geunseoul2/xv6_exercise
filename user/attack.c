#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // Your code here.
  
  // to attack that point, we need to call kalloc in attack..?
  // to call kalloc, we need to use sbrk() funtion.
  char* attack;
  attack = sbrk(PGSIZE);
  
  char* target = "This may help.";
  for(int i=0;i< PGSIZE; i++) {
    if(memcmp(attack+i, target, 14) == 0) {
      printf("%s\n",attack+i+16);
      exit(0);
    }
  }

  exit(1);
}
