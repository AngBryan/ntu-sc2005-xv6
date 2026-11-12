#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define MAX_SEMAPHORES 32

struct semaphore {
  struct spinlock lock;
  int value;
  int in_use;
};

struct semaphore semaphores[MAX_SEMAPHORES];
struct spinlock sem_table_lock;

void
seminit(void)
{
  initlock(&sem_table_lock, "sem_table");
  for(int i = 0; i < MAX_SEMAPHORES; i++) {
    semaphores[i].in_use = 0;
    initlock(&semaphores[i].lock, "semaphore");
  }
}

int
sem_alloc(int value)
{
  acquire(&sem_table_lock);
  for(int i = 0; i < MAX_SEMAPHORES; i++) {
    if(!semaphores[i].in_use) {
      semaphores[i].in_use = 1;
      semaphores[i].value = value;
      release(&sem_table_lock);
      return i;
    }
  }
  release(&sem_table_lock);
  return -1;  // No free semaphores
}

int
sem_free(int semid)
{
  if(semid < 0 || semid >= MAX_SEMAPHORES)
    return -1;
  
  acquire(&sem_table_lock);
  semaphores[semid].in_use = 0;
  release(&sem_table_lock);

  return 0;
}

int
sem_wait(int semid)
{
  if(semid < 0 || semid >= MAX_SEMAPHORES || !semaphores[semid].in_use)
    return -1;
 
  // TODO Retrieve semaphore: uncomment below
  struct semaphore *sem = &semaphores[semid];
  acquire(&sem->lock); // NOT sem_table_lock that is for allocation table. &(sem->lock) address of sem's lock 
  // TODO sleep while semaphore is zero
  while(sem->value==0){
    sleep(sem, &sem->lock); //sem is a pointer so we want to give it the address
  }
  // TODO decrement semaphore value atomically
  sem->value--;
  release(&sem->lock);
  return 0;
}

int
sem_signal(int semid)
{
  if(semid < 0 || semid >= MAX_SEMAPHORES || !semaphores[semid].in_use)
    return -1;

  // TODO Retrieve semaphore: uncomment below
  struct semaphore *sem = &semaphores[semid];
  acquire(&sem->lock); //REMEMBER TO ACQUIRE AND RELEASE
  // TODO increment semaphore value atomically, notifying sleeping processes
  sem->value++;
  wakeup(sem); //wakeup the chem from earlier ie sem NOT &sem->lock
  release(&sem->lock);
  return 0;
}

int
sem_getvalue(int semid)
{
  if(semid < 0 || semid >= MAX_SEMAPHORES || !semaphores[semid].in_use)
    return -1;
  // TODO return value of semid atomically
  acquire(&semaphores[semid]);
  int value = (&semaphores[semid])->value;
  release(&semaphores[semid]);
  return value;
}
