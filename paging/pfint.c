/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

extern int currpid;
extern fr_map_t g_frame_table[NFRAMES];


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{

  kprintf("Handle the page fault!\n");
  STATWORD ps;
  disable(ps);
  //read the Page Fault Linear Address
  unsigned long addr=read_cr2();
  //find the pde and pte
  pd_t* pde=(pd_t*)proctab[currpid].pdbr;
  pde+=(addr>>22);
  if(pde->pd_pres==0)
  {
    int new_pt=create_page_table(currpid);
    if(new_pt==SYSERR)
    {
      return SYSERR;
    }
    //now this pde is present
    pde->pd_pres=1;
    //update the vpn of this page
    pde->pd_base=new_pt+FRAME0;
  }
  pt_t* pte=(pt_t*)(pde->pd_base<<12);
  pte+=(addr>>12)&0x000003ff;
  //if a page falut occur, the pte->pres must be 0, we dont need to check
  pte->pt_pres=1;
  int* frame_num;
  int res=get_frm(frame_num);
  if(res==SYSERR)
  {
    return SYSERR;
  }
  //update the pte and the frame info
  pte->pt_base=*frame_num+FRAME0;
  g_frame_table[*frame_num].fr_status=FRM_MAPPED;
  g_frame_table[*frame_num].fr_pid=currpid;
  g_frame_table[*frame_num].fr_vpno=(addr<<12);
  g_frame_table[*frame_num].fr_refcnt++;
  g_frame_table[*frame_num].fr_type=FR_PAGE;
  g_frame_table[*frame_num].fr_dirty=1;
  //update the pd base register
  write_cr3(proctab[currpid].pdbr); 
  return OK;
}


