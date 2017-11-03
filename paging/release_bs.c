#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern bs_map_t g_back_store_table[BS_NUM];

SYSCALL release_bs(bsd_t bs_id) {
  /* release the backing store with ID bs_id */
  kprintf("PID:%d release_bsbs_id:%d\n",currpid,bs_id);
  if(g_back_store_table[bs_id].bs_status==BSM_UNMAPPED)
  {
    return SYSERR;
  }
  else
  {
    g_back_store_table[bs_id].bs_status=BSM_UNMAPPED;
    g_back_store_table[bs_id].bs_pid=-1;
  }
  return OK;
}

