/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

//the global table of frames for all the phiscial memory frame we have
fr_map_t g_frame_table[NFRAMES];
extern int page_replace_policy;
extern struct fr_queue_node* fr_queue_now;
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
  //find a free frame to return
  for(i=0;i<NFRAMES;i++)
  {
    if(g_frame_table[i].fr_status==FRM_UNMAPPED)
    {
      kprintf("PID:%d get_frm avail:%d\n",currpid,i);
      *avail=i;
      return OK;
    }
  }
  
  //else, swap a frame to return
  if(page_replace_policy==SC)
  {
    if(fr_queue_now==NULL)
      return SYSERR;
    struct fr_queue_node* temp;
    int frame_num;
    int vpno;
    pd_t* pde;
    pt_t* pte;
    while(1)
    {
      frame_num=fr_queue_now->frame_num;
      vpno=g_frame_table[frame_num].fr_vpno;
      pde=(pd_t*)proctab[currpid].pdbr;
      pde+=(vpno>>10);
      if(pde->pd_pres==0)
        return SYSERR;
      pte=(pt_t*)(pde->pd_base<<12);
      pte+=(vpno)&0x000003ff;
      if(pte->pt_pres==0)
        return SYSERR;
      //if the acc bit of the correctsponding page is 0, swap this frame
      if((pte->pt_acc)==0)
      {
        *avail=fr_queue_now->frame_num;
        temp->next=fr_queue_now->next;
        free_frm(fr_queue_now->frame_num);
        freemem(fr_queue_now,sizeof(struct fr_queue_node));
        fr_queue_now=temp->next;
        kprintf("PID:%d replace[SC]\n frame_num:%d",currpid,*avail);
        return OK;
      }
      //else set that acc bit to 0
      else
        pte->pt_acc=0;
      //move to the next frame in the queue
      temp=fr_queue_now;
      fr_queue_now=fr_queue_now->next;
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



