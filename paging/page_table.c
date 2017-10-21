#include <conf.h>
#include <proc.h>
#include <kernel.h>
#include <paging.h>

//The 4th enteries in all page directories are the same, so use 
//the global variable to save them
pt_t* g_general_page_table[4];

int create_page_table(int pro_id);
int init_general_page_table();
int create_page_dir();

int create_page_dir(int pro_id)
{
  int frame_num;
  int res=get_frame(frame_num);
  if(res!=OK)
  {
    return SYSERR;
  }
  
}

int create_page_table(int pro_id)
{
  int *frame_num;
  //find one empty frame to save the new page table
  int res=get_fem(frame_num);
  if(res!=OK)
  {
    return SYSERR;
  }
  //initialize all the page enteries, But the page in this table is not persent
  //yet, the page will be persent when a page fault occur
  int i;
  for(i=0;i<NFRAMES;i++)
  {
    pt_t* page_entery=(FRAME0+(*frame_num))*NBPG+sizeof(pt_t);
    //when initialize, the page entery is not present, maybe this will 
    //cause the page fault
    page_entery->pt_pres=0;
    page_entery->pt_base=0;
    page_entery->pt_avail=0;
    page_entery->pt_global=0;
    page_entery->mbz=0;
    page_entery->pt_dirty=0;
    page_entery->pt_acc=0;
    page_entery->pt_pcd=0;
    page_entery->pwt=0;
    page_entery->pt_user=0;
    page_entery->pt_write=1;
  }
  //This frame is used
  g_frame_table[*frame_num].fr_status=FRM_MAPPED;
  //Used as page table
  g_frame_table[*frame_num].fr_type=FR_TBL;
  g_frame_table[*frame_num].fr_pid=pro_id;
  g_frame_table[*frame_num].fr_dirty=0;
  g_frame_table[*frame_num].fr_loadtime=-1;
  g_frame_table[*frame_num].fr_refcnt=0;
  //return the frame number which contain the page table
  return *frame_num;
}

int init_general_page_table()
{
  int i;
  int frame_num;
  for(i=0;i<4;i++)
  {
    //the general page table is in control of null thread
    frame_num=create_page_table(0);
    //this pointer only contain the top 20 bit of the address
    general_page_table[i]=frame_num+FRAME0;
  }
  int ii;
  for(i=0;i<4;i++)
  {
    for(ii=0;ii<NFRAMES;ii++)
    {
      pt_t* page_entery=general_page_table[i]*NBPG+ii*sizeof(pt_t);
      //page in the general page table is present since the top 16MB is valid
      page_entery->pt_pres=1;
      //pt_base is the top 20 bit of the address
      //i is the PD index, ii is the PT index
      page_entery->pt_base=i<<10+ii;
      page_entery->pt_write=1;
      g_frame_tbale[frame_num].fr_refcnt++;
    }
  }
  return OK;
}
