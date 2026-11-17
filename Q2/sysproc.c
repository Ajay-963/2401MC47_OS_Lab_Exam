#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_mprotect(void)
{
  int addr, len;
  if(argint(0, &addr) < 0 || argint(1, &len) < 0)
    return -1;
  if(update_page_permissions(myproc()->pgdir, myproc()->sz,
                             (uint)addr, len, 0) < 0)
    return -1;
  lcr3(V2P(myproc()->pgdir));
  return 0;
}

int
sys_munprotect(void)
{
  int addr, len;
  if(argint(0, &addr) < 0 || argint(1, &len) < 0)
    return -1;
  if(update_page_permissions(myproc()->pgdir, myproc()->sz,
                             (uint)addr, len, 1) < 0)
    return -1;
  lcr3(V2P(myproc()->pgdir));
  return 0;
}

int
sys_set_priority(void)
{
  int pid, priority;
  if(argint(0, &pid) < 0 || argint(1, &priority) < 0)
    return -1;
  return set_priority(pid, priority);
}

// Assignment 9: print every valid user page in the current process's page table.
int
sys_vmprint(void)
{
  struct proc *p = myproc();
  pde_t *pgdir = p->pgdir;
  pte_t *pgtab;
  uint i, j, userpages = 0, pgtables = 0;
  uint vaddr, pa;

  cprintf("pid %d vmprint\n", p->pid);
  for(i = 0; i < NPDENTRIES; i++){
    if((pgdir[i] & PTE_P) == 0)
      continue;
    if(i >= (KERNBASE >> PDXSHIFT))
      continue;
    pgtables++;
    pgtab = (pte_t*)P2V(PTE_ADDR(pgdir[i]));
    for(j = 0; j < NPTENTRIES; j++){
      if((pgtab[j] & (PTE_U|PTE_W|PTE_P)) == 0)
        continue;
      vaddr = (i << PDXSHIFT) | (j << PTXSHIFT);
      if(vaddr >= KERNBASE)
        continue;
      pa = (pgtab[j] & PTE_P) ? PTE_ADDR(pgtab[j]) : 0;
      cprintf("VA 0x%x PA 0x%x P=%d W=%d U=%d\n",
              vaddr, pa,
              (pgtab[j] & PTE_P) ? 1 : 0,
              (pgtab[j] & PTE_W) ? 1 : 0,
              (pgtab[j] & PTE_U) ? 1 : 0);
      if((pgtab[j] & (PTE_P|PTE_U)) == (PTE_P|PTE_U))
        userpages++;
    }
  }
  cprintf("present user pages: %d\n", userpages);
  cprintf("page-directory pages: 1\n");
  cprintf("user page-table pages: %d\n", pgtables);
  return 0;
}

// Assignment 9: return the current process's handled page-fault count.
int
sys_getfaults(void)
{
  return myproc()->pagefaults;
}

int
sys_memstats(void)
{
  uint allocated, freed, current, available;

  kallocstats(&allocated, &freed, &current, &available);
  cprintf("allocated pages: %d\n", allocated);
  cprintf("freed pages: %d\n", freed);
  cprintf("currently allocated pages: %d\n", current);
  cprintf("available physical memory pages: %d\n", available);
  return 0;
}

int
sys_vtop(void)
{
  int address;
  pte_t *pte;
  struct proc *p = myproc();

  if(argint(0, &address) < 0 || (uint)address >= p->sz ||
     (uint)address >= KERNBASE)
    return -1;
  pte = walkpgdir(p->pgdir, (char*)address, 0);
  if(pte == 0 || (*pte & (PTE_P|PTE_U)) != (PTE_P|PTE_U))
    return -1;
  return PTE_ADDR(*pte) | ((uint)address & (PGSIZE - 1));
}

int
sys_pageinfo(void)
{
  int address;
  pte_t *pte;
  struct proc *p = myproc();

  if(argint(0, &address) < 0 || (uint)address >= p->sz ||
     (uint)address >= KERNBASE)
    return -1;
  pte = walkpgdir(p->pgdir, (char*)address, 0);
  if(pte == 0 || (*pte & (PTE_P|PTE_U)) != (PTE_P|PTE_U))
    return -1;
  cprintf("VA 0x%x frame %d P=%d W=%d U=%d\n",
          (uint)address, PTE_ADDR(*pte) >> PTXSHIFT,
          (*pte & PTE_P) != 0, (*pte & PTE_W) != 0,
          (*pte & PTE_U) != 0);
  return 0;
}

int
sys_setreadonly(void)
{
  int address;
  pte_t *pte;
  struct proc *p = myproc();

  if(argint(0, &address) < 0 || (uint)address >= p->sz ||
     (uint)address >= KERNBASE)
    return -1;
  pte = walkpgdir(p->pgdir, (char*)address, 0);
  if(pte == 0 || (*pte & (PTE_P|PTE_U)) != (PTE_P|PTE_U))
    return -1;
  *pte &= ~PTE_W;
  lcr3(V2P(p->pgdir));
  return 0;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
