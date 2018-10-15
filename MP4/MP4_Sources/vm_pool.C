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
#include "page_table.H"
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
  // status bits for writing to the page directory and page table
  unsigned long valid_bit;
  unsigned long read_or_write;
  unsigned long user_or_kernel;
  unsigned long use_bit;
  unsigned long dirty_bit;
  // buffer the inputs to private variables
  base_address = _base_address;
  size = _size;
  frame_pool = _frame_pool;
  page_table = _page_table;
  machine_page_size = Machine::PAGE_SIZE;
  free_size = size;
  num_allocated_regions = 0;
  // register the vmpool with PageTable Class
  page_table->register_pool(this);
  // Caluculate the size of virtual memory pool in terms of paper
  size_in_frames = (size/machine_page_size)+((size%machine_page_size)>0)?1:0;
  // Leave aside a frame base_address to store
  if(size_in_frames > 1)
  {
    unsigned long page_table_frame_number = frame_pool->get_frames(1);
    unsigned long data_frame_number = frame_pool->get_frames(1);
    unsigned long page_directory_entry = (base_address & (0xFFC00000))>>22;
    unsigned long page_table_entry = (base_address & (0x003FF000))>>12;
    // Make the entries into page_directory and page table by recursive lookup
    valid_bit = 0x00000001;
    read_or_write = 0x00000001;
    user_or_kernel = 0x00000000;
    use_bit = 0x00000000;
    dirty_bit = 0x00000000;
    *((unsigned long*)((1023<<22)|(1023<<12)|(page_directory_entry<<2)|0)) = (page_table_frame_number*machine_page_size)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
    *((unsigned long*)((1023<<22)|(page_directory_entry<<12)|(page_table_entry<<2)|0)) = (data_frame_number*machine_page_size)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
    free_size = size - machine_page_size;
    // Now that we have a mapping we can go a head and use the first frame for storing the list of regions allocated
    allocated_region_starting_address = (unsigned long*)(base_address);
    allocated_region_size = (unsigned long*)(base_address+((machine_page_size)/2));
    // Make the first entry in the regions occupied list
    *(allocated_region_starting_address) = base_address;
    *(allocated_region_size) = machine_page_size;
    num_allocated_regions++;
  }
  Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size)
{
  // Declare the required variables
  unsigned long available_base_address;
  // get the available base address from the array
  available_base_address = *(allocated_region_starting_address+num_allocated_regions-1) + *(allocated_region_size+num_allocated_regions-1);
  // Update the array with this new ENTRY
  *(allocated_region_starting_address+num_allocated_regions) = available_base_address;
  *(allocated_region_size+num_allocated_regions) = _size;
  num_allocated_regions++;
  Console::puts("Allocated region of memory of size ");Console::putui(_size);Console::puts("from address ");Console::putui(available_base_address);Console::puts("\n");
  // return the starting address of requested region
  return available_base_address;
}

void VMPool::release(unsigned long _start_address)
{
  unsigned int i;
  unsigned int j;
  unsigned int num_pages_occupied;
  unsigned long free_page_number;
  for(i=0;i<num_allocated_regions;i++)
  {
    if(*(allocated_region_starting_address+i) == _start_address)
    {
      num_pages_occupied = (*(allocated_region_size+i)/machine_page_size) +((*(allocated_region_size+i)%machine_page_size)>0)?1:0;
      for(j=0;j<num_pages_occupied;j++)
      {
        free_page_number = ((_start_address)>>12)+i;
      //  (*page_table)::free_page(free_page_number);
      }
      Console::puts("Released region of memory should be starting from ");Console::putui(*(allocated_region_starting_address+i));Console::puts(" with size of ");Console::putui(*(allocated_region_size+i));Console::puts("\n");
      *(allocated_region_starting_address+i) = 0;
      *(allocated_region_size+i) = 0;
      num_allocated_regions--;
    }
  }
  Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address)
{

  // check if the address falls in the range of vmpool or not
  if((_address>=base_address)&&(_address<(base_address+size)))
  {
    return true;
  }
  else
  {
    return false;
  }
  Console::puts("Checked whether address is part of an allocated region.\n");
}
