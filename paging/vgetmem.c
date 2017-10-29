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
  kprintf("Get mem from unallcated heap\n");
  STATWORD ps;
  disable(ps);
  nbytes=(unsigned int)roundmb(nbytes);
  struct mblock *a,*b,*new_addr;
  int remain;
  //read the current process mem list
  a=proctab[currpid].vmemlist;
  b=proctab[currpid].vmemlist->mnext;
  //go through all the free block and find one bigger than desire
  for(;b!=NULL;a=b,b=b->mnext)
  {
    if(b->mlen>=nbytes)
    {
      remain=b->mlen-nbytes;
      //if the block still has some remainning, init the node here
      if(remain!=0)
      {
        new_addr=(struct mblock*)((int*)b+nbytes);
        a->mnext=new_addr;
        new_addr->mnext=b->mnext;
        new_addr->mlen=remain;
      }
      //else just remove the node
      else
      {
        a->mnext=b->mnext;
      }
      restore(ps);
      return (WORD*)b;
    }
  }
  restore(ps);
  return SYSERR;
}


