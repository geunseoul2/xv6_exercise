#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
// CPU 하나(0번 CPU)가 시스템 전체 장치 초기화 / OS 필수 기반 세팅
void
main()
{
  if(cpuid() == 0){
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    kinit();         // physical page allocator
    kvminit();       // create kernel page table -> Direct Mapping?
    kvminithart();   // turn on paging
    procinit();      // process table
    trapinit();      // trap vectors
    trapinithart();  // install kernel trap vector
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    binit();         // buffer cache
    iinit();         // inode table
    fileinit();      // file table
    virtio_disk_init(); // emulated hard disk
    userinit();      // first user process
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}

/*
[ 높은 주소 (High Address / RAM의 끝 : 128MB ) ]
  +-------------------------------------------------+
  |                                                 |
  |   동적 할당 영역 (Free Memory)                    |
  |   - kinit()에 의해 4KB 페이지 단위로 관리됨         |  <--- kalloc()으로 유저 스택, 힙, 페이지 테이블,
  |   - 유저 프로세스가 띄워질 때마다 여기서 떼어줌       |       커널 스택(kstack)이 할당되는 공간!
  |                                                 |
  +-------------------------------------------------+ <--- end (커널 코드/데이터가 끝나는 지점)
  |   xv6 커널 데이터 및 BSS 영역 (.data, .bss)       |
  |   - proc[NPROC] (프로세스 테이블)                 |
  |   - stack0 (부팅용 임시 스택)                     |
  +-------------------------------------------------+
  |   xv6 커널 코드 영역 (.text, .rodata)             |  <--- main(), start(), sys_fork() 등의 C 코드
  +-------------------------------------------------+ <--- 0x80000000 (RISC-V RAM 시작 주소)
  |   I/O 장치 영역 (MMIO - UART, PLIC, CLINT 등)     |
  +-------------------------------------------------+ <--- 0x00000000
  [ 낮은 주소 (Low Address)] */