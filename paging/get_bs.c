#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern bs_map_t g_back_store_table[BS_NUM];

int get_bs(bsd_t bs_id, unsigned int npages) 
{

  /* requests a new mapping of npages with ID map_id */
  kprintf("Get a backing store mapping.\n");
  STATWORD ps;
  disable(ps);

  if(g_back_store_table[bs_id].bs_status==BSM_MAPPED)
  {
    kprintf("Mapped to a already used mapping.\n");
    restore(ps);
    return g_back_store_table[bs_id].bs_npages; 
  }
  else
  {
    kprintf("Mapped to a new backing store/\n");
    g_back_store_table[bs_id].bs_status=BSM_MAPPED;
    g_back_store_table[bs_id].bs_pid=currpid;
    restore(ps);
    return g_back_store_table[bs_id].bs_npages;
  }
  restore(ps);
  return SYSERR;

}


