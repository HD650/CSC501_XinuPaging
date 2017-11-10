/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

extern int currpid;
extern fr_map_t g_frame_table[NFRAMES];
extern int page_replace_policy;
extern struct fr_queue_node* fr_queue_head;
extern struct fr_queue_node* fr_queue_now;
extern struct fr_queue_node* fr_queue_end;

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
  STATWORD ps;
  disable(ps);
  //read the Page Fault Linear Address
  unsigned long addr=read_cr2();
#ifdef PG_DEBUG  
  kprintf("PID:%d page_fault vaddr:%x\n",currpid,addr);
#endif
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
  int frame_num;
  int res=get_frm(&frame_num);
  if(res==SYSERR)
  {
    return SYSERR;
  }
  //update the pte and the frame info
  pte->pt_base=frame_num+FRAME0;
  g_frame_table[frame_num].fr_status=FRM_MAPPED;
  g_frame_table[frame_num].fr_pid=currpid;
  g_frame_table[frame_num].fr_vpno=(addr<<12);
  g_frame_table[frame_num].fr_refcnt++;
  g_frame_table[frame_num].fr_type=FR_PAGE;
  //this frame is dirty since it's not the same one in the bs
  g_frame_table[frame_num].fr_dirty=1;
  
  //if the replace policy is SC, insert the frame to the cirular queue
  if(page_replace_policy==SC)
  {
    //the first page fault, init the head already existed
    if(fr_queue_head->frame_num==-1)
    {
      fr_queue_head->frame_num=frame_num;
      fr_queue_head->next=fr_queue_head;
      fr_queue_head->age=255;
      fr_queue_now=fr_queue_head;
      fr_queue_end=fr_queue_head;
    }
    else
    {
      struct fr_queue_node* new_node=(struct fr_queue_node*)getmem(sizeof(struct fr_queue_node));
      new_node->frame_num=frame_num;
      //make the queue a circular queue
      new_node->next=fr_queue_head;
      new_node->age=255;
      fr_queue_end->next=new_node;
      fr_queue_end=new_node;
    }
  }
  else if(page_replace_policy==AGING)
  {
    //the first time of page fault
    if(fr_queue_head->frame_num==-1)
    {
      fr_queue_head->frame_num=frame_num;
      fr_queue_head->next=NULL;
      fr_queue_head->age=255;
      fr_queue_now=fr_queue_head;
      fr_queue_end=fr_queue_head;
    }
    else
    {
      struct fr_queue_node* new_node=(struct fr_queue_node*)getmem(sizeof(struct fr_queue_node));
      new_node->frame_num=frame_num;
      new_node->next=NULL;
      new_node->age=255;
      fr_queue_end->next=new_node;
      fr_queue_end=new_node;
    }
  }

  //update the pd base register
  write_cr3(proctab[currpid].pdbr); 
  return OK;
}


