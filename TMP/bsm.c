/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

//record the 8 backing store info
bs_map_t g_back_store_table[BS_NUM];
//every process may have one or more backing store map
bs_map_t g_proc_bs_t[NPROC][BS_NUM];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
#ifdef PG_DEBUG
  kprintf("PID:%d init_bsm\n");
#endif
  int i;
  //only init the table entery here, no real mem allocate
  for(i=0;i<BS_NUM;i++)
  {
    g_back_store_table[i].bs_status=BSM_UNMAPPED;
    g_back_store_table[i].bs_pid=-1;
    //the vm of this backing store, it's in 16MB, so it's actully pm 
    g_back_store_table[i].bs_vpno=(BACKING_STORE_BASE+i*BACKING_STORE_UNIT_SIZE)>>12;
    //every backing store has 256 pages
    g_back_store_table[i].bs_npages=BACKING_STORE_UNIT_SIZE/NBPG;
    g_back_store_table[i].bs_count=0;
    g_back_store_table[i].bs_sem=-1;
  }
  //for every process
  for(i=0;i<NPROC;i++)
  {
    int ii;
    for(ii=0;ii<BS_NUM;ii++)
    {
      g_proc_bs_t[i][ii].bs_status=BSM_UNMAPPED;
      g_proc_bs_t[i][ii].bs_pid=i;
      //the vm mapped to this backing store 
      g_proc_bs_t[i][ii].bs_vpno=-1;
      //don't know the size now
      g_proc_bs_t[i][ii].bs_npages=-1;
      g_proc_bs_t[i][ii].bs_sem=-1; 
    }
  }
  return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
  int i;
  for(i=0;i<BS_NUM;i++)
  {
    if(g_back_store_table[i].bs_status==BSM_UNMAPPED)
      {
#ifdef PG_DEBUG
        kprintf("PID:%d get_bsm avail:%d\n",currpid,i);
#endif
        *avail=i;
        return i;
      }
  }
  return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
#ifdef PG_DEBUG
  kprintf("PID:%d free_bsm bsm_t:%d\n",currpid,i);
#endif
  //free the process mapping first
  int pid=g_back_store_table[i].bs_pid;
  g_proc_bs_t[pid][i].bs_status=BSM_UNMAPPED;
  g_proc_bs_t[pid][i].bs_vpno=-1;
  g_proc_bs_t[pid][i].bs_npages=-1;
  g_proc_bs_t[pid][i].bs_sem=-1;
  //free the backing store entery
  g_back_store_table[i].bs_status=BSM_UNMAPPED;
  g_back_store_table[i].bs_pid=-1;
  g_back_store_table[i].bs_sem=-1;
  return OK;
}

//util function for free a process bs map
SYSCALL free_proc_bsm(int pid)
{
#ifdef PG_DEBUG
  kprintf("PID:%d free_proc_bsm pid:%d\n",currpid,pid);
#endif
  int i;
  for(i=0;i<BS_NUM;i++)
  {
    //update the reference count of the bs table
    if(g_proc_bs_t[pid][i].bs_status==BSM_MAPPED)
      g_back_store_table[i].bs_count-=1;
    //free the proc bs table first
    g_proc_bs_t[pid][i].bs_status=BSM_UNMAPPED;
    g_proc_bs_t[pid][i].bs_vpno=-1;
    g_proc_bs_t[pid][i].bs_npages=-1;
    g_proc_bs_t[pid][i].bs_sem=-1;
    if(g_back_store_table[i].bs_count==0)
    {
      g_back_store_table[i].bs_status=BSM_UNMAPPED;
      g_back_store_table[i].bs_pid=-1;
      g_back_store_table[i].bs_sem=-1;
    }
  }
  return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
  int i;
  int min,max;
  //get the top 20 bit number
  unsigned int vpno=((unsigned long)vaddr)>>12;
  for(i=0;i<BS_NUM;i++)
  {
    if(g_proc_bs_t[pid][i].bs_status==BSM_UNMAPPED)
      continue;
    min=g_proc_bs_t[pid][i].bs_vpno;
    max=g_proc_bs_t[pid][i].bs_vpno+g_proc_bs_t[pid][i].bs_npages;
    //the address should in the range of the backing store
    if((vpno>=min)&&(vpno<max))
    {
#ifdef PG_DEBUG
      kprintf("PID:%d bsm_lookup vaddr:%x store:%d pageth:%d\n",currpid,vaddr,vpno-min);
#endif
      *store=i;
      *pageth=vpno-min;
      return OK;
    }
  }
  return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
  if(g_back_store_table[source].bs_status==BSM_MAPPED)
  {
  }
#ifdef PG_DEBUG  
kprintf("PID:%d bsm_map vpno:%x source:%d npages:%d\n",currpid,vpno,source,npages);
#endif
  g_back_store_table[source].bs_pid=pid;
  g_back_store_table[source].bs_status=BSM_MAPPED;
  g_back_store_table[source].bs_count+=1;
  g_proc_bs_t[pid][source].bs_status=BSM_MAPPED;
  g_proc_bs_t[pid][source].bs_vpno=vpno;
  g_proc_bs_t[pid][source].bs_npages=npages;
  return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
  int bs_num,page_num;
  //find this mapping
  int res=bsm_lookup(pid,vpno<<12,&bs_num,&page_num);
  if(res==SYSERR)
  {
    return SYSERR;
  }
  
  //save the data to the backing store
  int length=g_proc_bs_t[pid][bs_num].bs_npages;
  int i;
#ifdef PG_DEBUG
  kprintf("PID:%d bsm_unmap vpno:%x\n",currpid,vpno);
#endif  
  //use the info of this mapping to save mem to backing store
  for(i=0;i<length;i++)
    write_bs((vpno+i)<<12,bs_num,(page_num)+i);
  
  //unmap this in the data structure
  g_proc_bs_t[pid][bs_num].bs_status=BSM_UNMAPPED;
  g_proc_bs_t[pid][bs_num].bs_pid=-1;
  g_proc_bs_t[pid][bs_num].bs_vpno=-1;
  g_proc_bs_t[pid][bs_num].bs_npages=0;
  return OK;
}


