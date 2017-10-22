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
  kprintf("Initialize the frame table!\n");
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
  kprintf("Get a frame!\n");
  int i;
  for(i=0;i<NFRAMES;i++)
  {
    if(g_frame_table[i].fr_status==FRM_UNMAPPED)
    {
      *avail=i;
      return OK;
    }
  }
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  kprintf("To be implemented!\n");
  return OK;
}



