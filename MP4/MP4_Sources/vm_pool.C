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
  // Leave aside a frame base_address to store
  free_size = size - machine_page_size;
  // Hopefully the page_fault handler kicks in for the below accesses
  //allocated_region_starting_address = (unsigned long*)(base_address);
  //allocated_region_size = (unsigned long*)(base_address+((machine_page_size)/2));
  // Fill in the ordered pair to indicate that first page worthy of data is already occupied
  //*(allocated_region_starting_address) = base_address;
  //*(allocated_region_size) = machine_page_size;
  //num_allocated_regions++;
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
