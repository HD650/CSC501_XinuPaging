/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
#ifdef PG_DEBUG
  kprintf("PID:%d vgetmem nbytes:%d\n",currpid,nbytes);
#endif
  STATWORD ps;
  disable(ps);
  nbytes=(unsigned int)roundmb(nbytes);
  struct mblock *a,*b,*new_addr;
  int remain;
  //read the current process mem list
  //a=proctab[currpid].vmemlist;
  a=proctab[currpid].vmemlist->mnext;
  b=proctab[currpid].vmemlist;
  //go through all the free block and find one bigger than desire
  for(;a!=NULL;b=a,a=a->mnext)
  {
    if(b->mlen>=nbytes)
    {
      remain=b->mlen-nbytes;
      //if the block still has some remainning, init the node here
      if(remain!=0)
      {
        new_addr=(struct mblock*)(((int*)a)+nbytes);
        b->mnext=new_addr;
        new_addr->mnext=a->mnext;
        new_addr->mlen=remain;
      }
      //else just remove the node
      else
      {
        b->mnext=a->mnext;
      }
      restore(ps);
      return (WORD*)a;
    }
  }
  restore(ps);
  return SYSERR;
}


