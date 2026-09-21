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
  int reference_count[NPAGES]; //reference count of each physcial page
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  for(int i=0; i< NPAGES; i++) kmem.reference_count[i] = 1;
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  uint64 idx;
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  idx = PA_IDX(pa);

  acquire(&kmem.lock);
  if(kmem.reference_count[idx] > 1) { //other reference still exist
    kmem.reference_count[idx]--;
    release(&kmem.lock);
    return;
  }

  kmem.reference_count[idx]--;
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
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
  if(r) {
    kmem.freelist = r->next;
    kmem.reference_count[PA_IDX(r)] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

int
page_reference_count(uint64 pa) {
  int count;

  acquire(&kmem.lock);
  count = kmem.reference_count[PA_IDX(pa)];
  release(&kmem.lock);

  return count;
}

void
page_refcount_add(uint64 pa)
{
  acquire(&kmem.lock);
  kmem.reference_count[PA_IDX(pa)]++;
  release(&kmem.lock);
}

void
page_refcount_sub(uint64 pa)
{
  acquire(&kmem.lock);
  kmem.reference_count[PA_IDX(pa)]--;
  release(&kmem.lock);
}