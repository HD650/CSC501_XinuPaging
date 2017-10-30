/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

void halt();

#define TPASSED    1
#define TFAILED    0

#define MYVADDR1   0x40000000
#define MYVPNO1    0x40000
#define MYVADDR2   0x80000000
#define MYVPNO2    0x80000
#define MYBS1      1
#define MAX_BSTORE 16

#ifndef NBPG
#define NBPG       4096
#endif

#define assert(x,error) if(!(x)){ \
      kprintf(error);\
      return;\
      }
      
void test1()
{
  kprintf("\nTest 1: Testing xmmap.\n");
  char *addr1 = (char*)0x40000000; 
  int i = ((unsigned long)addr1) >> 12; 
  
  get_bs(MYBS1, 100);
  if (xmmap(i, MYBS1, 100) == SYSERR) {
    kprintf("xmmap call failed\n");
    kprintf("\tFAILED!\n");
    return;
  }

  for (i = 0; i < 26; i++) {
    *addr1 = 'A'+i;   //trigger page fault for every iteration
    addr1 += NBPG;    //increment by one page each time
  }

  addr1 = (char*)0x40000000;
  for (i = 0; i < 26; i++) {
    if (*addr1 != 'A'+i) {
      kprintf("\tFAILED!\n");
      return;
    }
    addr1 += NBPG;    //increment by one page each time
  }

  xmunmap(0x40000000>>12);
  release_bs(MYBS1);
  kprintf("\tPASSED!\n");
  return;
}
/*----------------------------------------------------------------*/
void proc_test2(int i,int j,int* ret,int s) {
  char *addr;
  int bsize;
  int r;
  bsize = get_bs(i, j);
  if (bsize != 50)
    *ret = TFAILED;
  r = xmmap(MYVPNO1, i, j);
  if (j<=50 && r == SYSERR){
    *ret = TFAILED;
  }
  if (j> 50 && r != SYSERR){
    *ret = TFAILED;
  }
  sleep(s);
  if (r != SYSERR)
    xmunmap(MYVPNO1);
  release_bs(i);
  return;
}
void test2() {
  int pids[16];
  int mypid;
  int i,j;

  int ret = TPASSED;
  kprintf("\nTest 2: Testing backing store operations\n");

  int bsize = get_bs(1, 100);
  if (bsize != 100)
    ret = TFAILED;
  release_bs(1);
  bsize = get_bs(1, 130);
  if (bsize != SYSERR)
    ret = TFAILED;
  bsize = get_bs(1, 0);
  if (bsize != SYSERR)
    ret = TFAILED;

  mypid = create(proc_test2, 2000, 20, "proc_test2", 4, 1,
                 50, &ret, 4);
  resume(mypid);
  sleep(2);
  for(i=1;i<=5;i++){
    pids[i] = create(proc_test2, 2000, 20, "proc_test2", 4, 1,
                     i*20, &ret, 0);
    resume(pids[i]);
  }
  sleep(3);
  kill(mypid);
  for(i=1;i<=5;i++){
    kill(pids[i]);
  }
  if (ret != TPASSED)
    kprintf("\tFAILED!\n");
  else
    kprintf("\tPASSED!\n");
}
/*-------------------------------------------------------------------------------------*/
void proc1_test3(int i,int* ret) {
  char *addr;
  int bsize;
  int r;

  get_bs(i, 100);
  
  if (xmmap(MYVPNO1, i, 100) == SYSERR) {
    *ret = TFAILED;
    return 0;
  }
  sleep(4);
  xmunmap(MYVPNO1);
  release_bs(i);
  return;
}
void proc2_test3() {
  /*do nothing*/
  sleep(1);
  return;
}
void test3() {
  int pids[16];
  int mypid;
  int i,j;

  int ret = TPASSED;
  kprintf("\nTest 3: Private heap is exclusive\n");

  for(i=0;i<=15;i++){
    pids[i] = create(proc1_test3, 2000, 20, "proc1_test3", 2, i,&ret);
    if (pids[i] == SYSERR){
      ret = TFAILED;
    }else{
      resume(pids[i]);
    }
  }
  sleep(1);
  mypid = vcreate(proc2_test3, 2000, 100, 20, "proc2_test3", 0, NULL);
  if (mypid != SYSERR)
    ret = TFAILED;

  for(i=0;i<=15;i++){
    kill(pids[i]);
  }
  if (ret != TPASSED)
    kprintf("\tFAILED!\n");
  else
    kprintf("\tPASSED!\n");
}

void test_proc()
{
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
}

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");
	test1();
	test2();
	test3();
	shutdown();
}
