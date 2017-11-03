/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

//the global table of frames for all the phiscial memory frame we have
fr_map_t g_frame_table[NFRAMES];

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  kprintf("PID:%d init_frm\n");
  int i;
  for(i=0;i<NFRAMES;i++)
  {
    g_frame_table[i].fr_status=FRM_UNMAPPED;
    g_frame_table[i].fr_pid=-1;
    g_frame_table[i].fr_vpno=-1;
    g_frame_table[i].fr_refcnt=0;
    g_frame_table[i].fr_type=FR_UNDEFINE;
    g_frame_table[i].fr_dirty=0;
  }
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  int i;
  for(i=0;i<NFRAMES;i++)
  {
    if(g_frame_table[i].fr_status==FRM_UNMAPPED)
    {
      kprintf("PID:%d get_frm avail:%d\n",currpid,i);
      *avail=i;
      return OK;
    }
  }
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
  kprintf("PID:%d free_frm frm_num:%d\n",currpid,i);
  //we only free frame that contain the page not page table
  if(g_frame_table[i].fr_type!=FR_PAGE)
  {
    return SYSERR;
  }
  int bs_num,page_num;
  fr_map_t frame=g_frame_table[i];
  int res=bsm_lookup(frame.fr_pid,frame.fr_vpno*NBPG,&bs_num,&page_num);
  //can't free a frame without backing store
  if(res==SYSERR)
  {
    return SYSERR;
  }
  //save this frame to the backing store
  write_bs((i+FRAME0)*NBPG,bs_num,page_num);
  //to free a frame ,we also need to free the page table entery
  pd_t* pde=(pd_t*)proctab[frame.fr_pid].pdbr;
  pde+=(frame.fr_vpno>>10);
  pt_t* pte=(pt_t*)(pde->pd_base<<12);
  pte+=(frame.fr_vpno)&0x000003ff;
  pte->pt_pres=0;
  pte->pt_write=0;
  pte->pt_dirty=0;
  //free the frame structure
  frame.fr_status=FRM_UNMAPPED;
  frame.fr_pid=-1;
  frame.fr_vpno=-1;
  frame.fr_dirty=0;
  return OK;
}



