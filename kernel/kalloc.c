// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

//Add Superpage freelist, and lock
struct superrun {
  struct superrun *next;
};

struct {
  struct spinlock lock;
  struct superrun *superfreelist;
} superkmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&superkmem.lock, "superkmem");
  freerange(end, (void*)PHYSTOP);
}

//Change the freerange to make 8 superpagefreelist first, and then make freelist.
void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  int n = 0;

  p = (char*)PGROUNDUP((uint64)pa_start);

  while ((uint64)p + PGSIZE <= (uint64)pa_end) {
    if (n < 8 && ((uint64)p % SUPERPGSIZE) == 0) {
      superfree(p);
      p += SUPERPGSIZE;
      n++;
      continue;
    }
    kfree(p);
    p += PGSIZE;
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Free the superpage like kfree
void
superfree(void* pa) {
  struct superrun *sr;

  if(((uint64)pa % SUPERPGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("superfree");

  memset(pa, 1, SUPERPGSIZE);

  sr = (struct superrun *)pa;

  acquire(&superkmem.lock);
  sr->next = superkmem.superfreelist;
  superkmem.superfreelist = sr;
  release(&superkmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// Allocate one 2-MB superpage of physical memory(just like kalloc)
void *
superalloc(void)
{
  struct superrun *sr;

  acquire(&superkmem.lock);
  sr = superkmem.superfreelist;
  if (sr)
    superkmem.superfreelist = sr->next;
  release(&superkmem.lock);

  if (sr)
    memset((char*)sr, 5, SUPERPGSIZE);
  return (void *)sr;
}