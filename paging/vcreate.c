/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
  kprintf("PID:%d vcreate\n",currpid);
  //a process with virtual heap should be a original process
  int proc_id=create(procaddr,ssize,priority,name,nargs,args);
  if(proc_id==SYSERR)
    return SYSERR;
  //all heap of process begins with virtual page number 4096
  proctab[proc_id].vhpno=V_HEAP;
  //save the heap size
  proctab[proc_id].vhpnpages=hsize;

  struct mblock *heap_begin=(struct mblock*)((V_HEAP)<<12);
  //when init, the heap is a whole chunk of free mem
  //heap_begin->mnext=NULL;
  //heap_begin->mlen=0;
  //the head of the linked list point to the fist free chunk of mem
  proctab[proc_id].vmemlist->mlen=NBPG*hsize;
  proctab[proc_id].vmemlist->mnext=heap_begin;
  return proc_id;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
