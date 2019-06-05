#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
  int i;
  //Case 1 If shared memory id is found
  acquire(&(shm_table.lock));//grab lock of the shm_table
  //table size = 64
  for(i = 0; i < 64; i++) {
    if(shm_table.shm_pages[i].id == id) {
      //increase reference count, then do mappages
      if (mappages(myproc()->pgdir, (char *)PGROUNDUP(myproc()->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U) == -1) {
        release(&(shm_table.lock));
        return -1;
      }
      shm_table.shm_pages[i].refcnt += 1;
      //*pointer=(char *)va;
      *pointer = (char*)PGROUNDUP(myproc()->sz);
      //add sz with page size
      myproc()->sz += PGSIZE;      
      release(&(shm_table.lock));
      return 0;
    }
  }
  //Case 2 If shared memory id is not found
  for(i = 0; i < 64; i++) {
    if(shm_table.shm_pages[i].id == 0) {
      shm_table.shm_pages[i].id = id;
      
      // Allocate a page using kalloc
      if ((shm_table.shm_pages[i].frame = kalloc()) == 0) {
        release(&(shm_table.lock));
        return -1;
      }
      memset(shm_table.shm_pages[i].frame, 0, PGSIZE);
      // Do the same as case 1. 
      if (mappages(myproc()->pgdir, (char *)PGROUNDUP(myproc()->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U) == -1) {
        release(&(shm_table.lock));
        return -1;
      }
      shm_table.shm_pages[i].refcnt = 1;
      *pointer = (char*)PGROUNDUP(myproc()->sz);
      //add sz with page size
      myproc()->sz += PGSIZE;      
      release(&(shm_table.lock));
      return 0;
    }
  }
  release(&(shm_table.lock));//if somehow it never hits the if statement
  return -1; //added to remove compiler warning -- you should decide what to return
}

int shm_close(int id) {
  int i;
  acquire(&(shm_table.lock));//grab lock of the shm_table
  for (i = 0; i < 64; i++) {
    if (shm_table.shm_pages[i].id == id) {
      shm_table.shm_pages[i].refcnt -= 1;

      if (shm_table.shm_pages[i].refcnt < 1) {        
        shm_table.shm_pages[i].id = 0;
        shm_table.shm_pages[i].frame = 0;
        shm_table.shm_pages[i].refcnt = 0;
      }

      release(&(shm_table.lock));
      return 0; 
    }
  }
  
  release(&(shm_table.lock));//if somehow it never hits the if statement
  return -1; //added to remove compiler warning -- you should decide what to return
}
