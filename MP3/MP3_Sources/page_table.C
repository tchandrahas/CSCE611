#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   // Initiate the paging system by copying the input kernel and process memory pools
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
  // declare the required variables
  unsigned long page_directory_frame_number;
  unsigned long page_table_frame_number;
  unsigned int i;
  // get 8 frame from kernel pool to store the page directory
  page_directory_frame_number = kernel_mem_pool->get_frames(8);
  // get 8 frames from kernel pool to store the page table of the first 4MB
  page_table_frame_number = kernel_mem_pool->get_frames(8);
  // set the entries of the page_table for the first 4MB
  for(i=0;i<1024;i++)
  {
    // Needs Review setting up the page table with identical frame number and setting the valid bit to 1
    *((page_table_frame_number*PAGE_SIZE)+i) = (i<<10)|(1);
  }
  // copy the page_table pointer to the page_directory at 00
  *((page_directory_frame_number*PAGE_SIZE)+0) = (page_table_frame_number*PAGE_SIZE);
  // copy the page_directory_frame_number into the page_directory variable of PageTable Class
  page_directory = page_directory_frame_number;
  Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   // Load the page directory number into the CR3 Register
   write_cr3(*page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   assert(false);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  assert(false);
  Console::puts("handled page fault\n");
}
