/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

//inited in initialize.c
extern int page_replace_policy;
extern struct fr_queue_node* fr_queue_head;
extern struct fr_queue_node* fr_queue_end;


/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  /* sanity check ! */

  kprintf("PID:%d srpolicy policy:%d\n",currpid,policy);
  page_replace_policy=policy;
  //the data structure is shared between AGING and SC, but a little different
  if(policy==SC)
    fr_queue_end->next=fr_queue_head;
  else if(policy==AGING)
    fr_queue_end->next=NULL;
  return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}
