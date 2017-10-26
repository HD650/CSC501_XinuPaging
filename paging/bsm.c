/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

//record the 8 backing store info
bs_map_t g_back_store_table[BS_NUM];
//every process may have one backing store map
bs_map_t g_proc_bs_t[NPROC];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
  kprintf("Initialize backing store mapping table.\n");
  int i;
  //only init the table entery here, no real mem allocate
  for(i=0;i<BS_NUM;i++)
  {
    g_back_store_table[i].bs_status=BSM_UNMAPPED;
    g_back_store_table[i].bs_pid=-1;
    //the vm of this backing store, it's in 16MB, so it's actully pm 
    g_back_store_table[i].bs_vpno=(BACKING_STORE_BASE+i*BACKING_STORE_UNIT_SIZE)<<12;
    //every backing store has 256 pages
    g_back_store_table[i].bs_npages=BACKING_STORE_UNIT_SIZE/NBPG;
    g_back_store_table[i].bs_sem=-1;
  }
  //for every process
  for(i=0;i<NPROC;i++)
  {
    g_proc_bs_t[i].bs_status=BSM_UNMAPPED;
    g_proc_bs_t[i].bs_pid=i;
    //the vm mapped to this backing store 
    g_proc_bs_t[i].bs_vpno=-1;
    //don't know the size now
    g_proc_bs_t[i].bs_npages=-1;
    g_proc_bs_t[i].bs_sem=-1; 
  }
  return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
  kprintf("Get a bsm.\n");
  int i;
  for(i=0;i<BS_NUM;i++)
  {
    if(g_back_store_table[i].bs_status==BSM_UNMAPPED)
      {
        //map this free back store
        g_back_store_table[i].bs_status=BSM_MAPPED;
        *avail=i;
        return i;
      }
  }
  kprintf("No free bsm, error...\n");
  return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
  kprintf("Free a bsm.\n");
  //free the process mapping first
  int pid=g_back_store_table[i].bs_pid;
  g_proc_bs_t[pid].bs_status=BSM_UNMAPPED;
  g_proc_bs_t[pid].bs_vpno=-1;
  g_proc_bs_t[pid].bs_npages=-1;
  g_proc_bs_t[pid].bs_sem=-1;
  //free the backing store entery
  g_back_store_table[i].bs_status=BSM_UNMAPPED;
  g_back_store_table[i].bs_pid=-1;
  g_back_store_table[i].bs_sem=-1;
  return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
  kprintf("Mapping a backing store to a process.\n");
  if(g_back_store_table[source].bs_status==BSM__UNMAPPED)
  {
    kprintf("The backing store is not get yet!error...\n");
    return SYSERR;
  }
  g_back_store_table[source].bs_pid=pid;
  //TODO alot
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
}


