#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern bs_map_t g_back_store_table[BS_NUM];

int get_bs(bsd_t bs_id, unsigned int npages) 
{

  /* requests a new mapping of npages with ID map_id */
  STATWORD ps;
  disable(ps);

  if(g_back_store_table[bs_id].bs_status==BSM_MAPPED)
  {
    kprintf("PID:%d get_bs BSM_MAPPED  bsid:%d\n",currpid,bs_id);
    restore(ps);
    //return g_back_store_table[bs_id].bs_npages;
    return npages; 
  }
  else
  {
    kprintf("PID:%d get_bs BSM_UNMAPPED bs_id%d\n",currpid,bs_id);
    g_back_store_table[bs_id].bs_status=BSM_MAPPED;
    g_back_store_table[bs_id].bs_pid=currpid;
    restore(ps);
    //return g_back_store_table[bs_id].bs_npages;
    return npages;
  }
  restore(ps);
  return SYSERR;

}


