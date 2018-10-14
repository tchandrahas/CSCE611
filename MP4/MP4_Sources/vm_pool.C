/*
 File: vm_pool.C

 Author:
 Date  :

 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address, unsigned long  _size, ContFramePool *_frame_pool, PageTable *_page_table)
{
  // buffer the inputs to private variables
  base_address = _base_address;
  size = _size;
  frame_pool = _frame_pool;
  page_table = _page_table;
  machine_page_size = page_table->PAGE_SIZE;
  // register the vmpool with PageTable Class
  page_table->register(this);
  Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size)
{
  // Declare the required variables
  unsigned long physical_frame_number;
  unsigned long page_table_entry;
  // buffer the input _size to a local variable
  unsigned int num_bytes = _size;
  // caluculate the number of frames needed for the given size
  unsigned int num_frames;
  if((num_bytes%machine_page_size)>0)
  {
    num_frames = (num_bytes/machine_page_size)+1;
  }
  else
  {
    num_frames = (num_bytes/machine_page_size);
  }
  // We get the physical frames from the input physical frame pool
  physical_frame_number = frame_pool.get_frames(num_frames);
  // Do Error handling for get_frames() function failure
  if(physical_frame_number < 0)
  {
    Console::puts("Unable to find physical frames, Check the code !!\n");
  }
  Console::puts("Got the starting physical frame as");Console::putui(physical_frame_number);Console::puts("for the request\n");
  // Make a page table entry for newly allocated frame
  // first caluculate the page table entry with base_address
  base_page_table_entry = ((base_address & 0x003FF000)>>12);
  for(unsigned int i=0;i<num_frames;i++)
  {
    *()
  }
  Console::puts("Allocated region of memory.\n");
}

void VMPool::release(unsigned long _start_address)
{
    assert(false);
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    assert(false);
    Console::puts("Checked whether address is part of an allocated region.\n");
}
