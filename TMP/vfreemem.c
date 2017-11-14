/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
#ifdef PG_DEBUG
  kprintf("PID:%d vfreemem block:%x size:%d\n",currpid,block,size);
#endif
  STATWORD ps;
  disable(ps);
  int nbytes=(unsigned int)roundmb(nbytes);
  struct mblock *a,*b,*new_addr;
  a=proctab[currpid].vmemlist->mnext;
  b=proctab[currpid].vmemlist;
  //find the nearest free mem location of block 
  for(;a!=NULL;b=a,a=a->mnext)
  {
    if(a>=block)
      break;
  }
  unsigned int top=b->mlen+b;
  //if some of the ready to free mem is inside a free mem
  if((block>b)&&(block<top))
  {
    return SYSERR;
  }
  if((size+block)>a)
  {
    return SYSERR;
  }
  //if the ready to free mem is just above a block, extend the existed node
  if((unsigned int)block==top)
  {
    b->mlen+=size;
  }
  //else, just insert the new node inside the mem list
  else
  {
    block->mlen=size;
    block->mnext=a;
    b->mnext=block;
  }
  restore(ps);
  return(OK);
}
