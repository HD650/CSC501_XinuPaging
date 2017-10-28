/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern bs_map_t g_back_store_table[BS_NUM];

/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  kprintf("xmmap - Mapping vm to bs.\n");
  STATWORD ps;
  disable(ps);

  if(g_back_store_table[source].bs_status==BSM_UNMAPPED)
  {
    kprintf("Mapped a un mapped backstore,erro...\n");
    restore(ps);
    return SYSERR;
  }
  else
  {
    int res=bsm_map(currpid,virtpage,source,npages);
    if(res==SYSERR)
    {
      restore(ps);
      return SYSERR;
    }
  }
  restore(ps);
  return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  kprintf("Unmap a bs mapping.\n");
  int res=bsm_unmap(currpid,virtpage,-1);
  if(res==SYSERR)
    return SYSERR;
  return OK;
}
