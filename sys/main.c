/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");
	//test whether the page fault work
  	int *test=0xffffffff;
  	*test=1;
  	kprintf("test the page fault handler: test:%d\n",*test);
        char *x; 
        char temp; 
        get_bs(4, 100); 
        xmmap(7000, 4, 100);    /* This call simply creates an entry in the backing store mapping */ 
        x = 7000*4096; 
        *x = 'Y';                            /* write into virtual memory, will create a fault and system should proceed as in the prev example */ 
        temp = *x;                        /* read back and check */ 
        xmunmap(7000); 
	shutdown();
}
