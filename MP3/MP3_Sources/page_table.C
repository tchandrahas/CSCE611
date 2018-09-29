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
  unsigned long i;
  // Declare local variables for status bits in page table/page directory entry
  /*
    valid_bit: (position=0) 1->valid, 0->invalid
    read_or_write: (position=1) 0->read only, 1->read-write
    user_or_kernel:(position=2) 0->user, 1->kernel
    use_bit:(position=4) 0->not used, 1->used
    dirty_bit:(position=5) 0->not dirty, 1-> dirty
  */
  unsigned long valid_bit;
  unsigned long read_or_write;
  unsigned long user_or_kernel;
  unsigned long use_bit;
  unsigned long dirty_bit;
  // get 8 frame from kernel pool to store the page directory
  page_directory_frame_number = process_mem_pool->get_frames(8);
  // do error handling for page_directory_frame_number
  if(page_directory_frame_number == 0)
  {
    Console::puts("Unable to get a physical memory frame for page directory storage\n");
    assert(false);
  }
  else
  {
    Console::puts("Got physical frame number ");Console::putui(page_directory_frame_number);Console::puts(" for page_directory\n");
  }
  // get 8 frames from kernel pool to store the page table of the first 4MB
  page_table_frame_number = process_mem_pool->get_frames(8);
  // do error handling for page_table_frame_number
  if(page_table_frame_number == 0)
  {
    Console::puts("Unable to get a physcial memory frame for page table storage\n");
    assert(false);
  }
  else
  {
    Console::puts("Got physical frame number ");Console::putui(page_table_frame_number);Console::puts(" for page table\n");
  }
  // set the entries of the page_table for the first 4MB
  for(i=0;i<1024;i++)
  {
    // Needs Review setting up the page table with identical frame number and setting the valid bit to 1
    valid_bit = 0x00000001;
    read_or_write = 0x00000001;
    user_or_kernel = 0x00000000;
    use_bit = 0x00000000;
    dirty_bit = 0x00000000;
    *((unsigned long*)((page_table_frame_number*PAGE_SIZE)+(4*i))) = (i*PAGE_SIZE)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit) ;
    Console::puts("Page Table Address: ");Console::putui((unsigned int)((page_table_frame_number*PAGE_SIZE)+(4*i)));Console::puts(" Entry: "); Console::putui(*((unsigned long*)((page_table_frame_number*PAGE_SIZE)+(4*i))));Console::puts("\n");
  }
  // copy the page_table pointer to the page_directory at 00
  valid_bit = 0x00000001;
  read_or_write = 0x00000001;
  user_or_kernel = 0x00000000;
  use_bit = 0x00000000;
  dirty_bit = 0x00000000;
  *((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+0)) = (page_table_frame_number*PAGE_SIZE)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
  Console::puts("Page Directory Address: ");Console::putui(((unsigned int)((page_directory_frame_number*PAGE_SIZE)+(4*0))));Console::puts(" Entry: ");Console::putui(*((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+(4*0))));
  // Set the Rest of the page directory entries to be invalid
  for(i=1;i<1024;i++)
  {
    valid_bit = 0x00000000;
    read_or_write = 0x00000001;
    user_or_kernel = 0x0000000;
    use_bit = 0x00000000;
    dirty_bit = 0x00000000;
    *((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+(4*i))) = (0<<20)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
    //Console::puts("Page Directory Address: ");Console::putui(((unsigned int)((page_directory_frame_number*PAGE_SIZE)+(4*i))));Console::puts(" Entry: ");Console::putui(*((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+(4*i))));
  }
  // copy the page_directory_frame_number into the page_directory variable of PageTable Class
  page_directory = (unsigned long*)((page_directory_frame_number*PAGE_SIZE));
  Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   // Load the page directory number into the CR3 Register
   write_cr3((unsigned long)page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
  paging_enabled = 1;
  // set the enable_paging to 1
  unsigned long cr0_read = read_cr3();
  // Set particular bit of CR0 to start paging, not sure about this
  write_cr0(cr0_read|(1<<31)|(1));
  Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  // copy the error code from REGS data structure
  unsigned int error_code = _r->err_code;
  Console::puts("Received an exception with error code ");Console::putui(error_code);Console::puts("\n");
  assert(false);
  // find the reason for exception

  Console::puts("handled page fault\n");
}
