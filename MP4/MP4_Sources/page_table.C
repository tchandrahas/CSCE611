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
  // get 1 frame from process memory pool to store the page directory
  page_directory_frame_number = process_mem_pool->get_frames(1);
  // do error handling for page_directory_frame_number
  if(page_directory_frame_number == 0)
  {
    Console::puts("Unable to get a physical memory frame for page directory storage\n");
    assert(false);
  }
  /*else
  {
    Console::puts("Got physical frame number ");Console::putui(page_directory_frame_number);Console::puts(" for page_directory\n");
  }*/
  // get 1 frames from process  pool to store the page table of the first 4MB
  page_table_frame_number = process_mem_pool->get_frames(1);
  // do error handling for page_table_frame_number
  if(page_table_frame_number == 0)
  {
    Console::puts("Unable to get a physcial memory frame for page table storage\n");
    assert(false);
  }
  /*else
  {
    Console::puts("Got physical frame number ");Console::putui(page_table_frame_number);Console::puts(" for page table\n");
  }*/
  // set the entries of the page_table for the first 4MB
  for(i=0;i<1024;i++)
  {
    // Needs Review setting up the page table with identical frame number and setting the valid bit to 1
    valid_bit = 0x00000001;
    read_or_write = 0x00000001;
    user_or_kernel = 0x00000001;
    use_bit = 0x00000000;
    dirty_bit = 0x00000000;
    *((unsigned long*)((page_table_frame_number*PAGE_SIZE)+(4*i))) = (i*PAGE_SIZE)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit) ;
    //Console::puts("Page Table Address: ");Console::putui((unsigned int)((page_table_frame_number*PAGE_SIZE)+(4*i)));Console::puts(" Entry: "); Console::putui(*((unsigned long*)((page_table_frame_number*PAGE_SIZE)+(4*i))));Console::puts("\n");
  }
  // copy the page_table pointer to the page_directory at 00
  valid_bit = 0x00000001;
  read_or_write = 0x00000001;
  user_or_kernel = 0x00000001;
  use_bit = 0x00000000;
  dirty_bit = 0x00000000;
  *((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+0)) = (page_table_frame_number*PAGE_SIZE)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
  //Console::puts("Page Directory Address: ");Console::putui(((unsigned int)((page_directory_frame_number*PAGE_SIZE)+(4*0))));Console::puts(" Entry: ");Console::putui(*((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+(4*0))));Console::puts("\n");
  // Set the Rest of the page directory entries to be invalid
  for(i=1;i<1024;i++)
  {
    valid_bit = 0x00000000;
    read_or_write = 0x00000001;
    user_or_kernel = 0x0000001;
    use_bit = 0x00000000;
    dirty_bit = 0x00000000;
    *((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+(4*i))) = (0<<20)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
    //Console::puts("Page Directory Address: ");Console::putui(((unsigned int)((page_directory_frame_number*PAGE_SIZE)+(4*i))));Console::puts(" Entry: ");Console::putui(*((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+(4*i))));
  }
  valid_bit = 0x00000001;
  read_or_write = 0x00000001;
  user_or_kernel = 0x0000000;
  use_bit = 0x00000000;
  dirty_bit = 0x00000000;
  // Make the upper most entry in paging directory to point to the starting of page directory
  *((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+(4*1023))) = (page_directory_frame_number*PAGE_SIZE)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
  //Console::puts("The Page directory starts at the address ");Console::putui((page_directory_frame_number*PAGE_SIZE));Console::puts("\n");
  //Console::puts("Last entry in the page directory is placed at address ");Console::putui((unsigned long)((page_directory_frame_number*PAGE_SIZE)+(4*1023)));Console::puts(" with value ");Console::putui(*((unsigned long*)((page_directory_frame_number*PAGE_SIZE)+(4*1023))));Console::puts("\n");
  // copy the page_directory_frame_number into the page_directory variable of PageTable Class
  page_directory = (unsigned long*)((page_directory_frame_number*PAGE_SIZE));
  //Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   // Load the page directory number into the CR3 Register
   unsigned int cr3_read = read_cr3();
   write_cr3(cr3_read|(unsigned long)page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
  paging_enabled = 1;
  // set the enable_paging to 1
  unsigned long cr0_read = read_cr0();
  // Set particular bit of CR0 to start paging, not sure about this
  write_cr0(cr0_read|(1<<31)|(1));
  Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  // copy the error code from REGS data structure
  unsigned int error_code = _r->err_code;
  //Console::puts("Received an exception with error code ");Console::putui(error_code);Console::puts("\n");
  // find the address that caused the exception
  unsigned long fault_virtual_address = (read_cr2());
  //Console::puts("Faulty Virtual Address is ");Console::putui(fault_virtual_address);Console::puts("\n");
  // get the page directory address
  unsigned long local_page_directory = (read_cr3() & 0xFFFFF000);
  //Console::puts("Local Page Directory is found to be at ");Console::putui(local_page_directory);Console::puts("\n");
  // Reason for fault
  unsigned long fault_page_directory_offset = ((fault_virtual_address & 0xFFC00000)>>22);
  unsigned long* fault_page_directory_entry = (unsigned long*)((1023<<22)|(1023<<12)|(fault_page_directory_offset<<2));
  //Console::puts("Faulty Page Directory Entry found at ");Console::putui((unsigned long) fault_page_directory_entry);Console::puts("\n");
  unsigned long fault_page_table_offset;
  unsigned long fault_page_table_addr;
  unsigned long directory_valid_bit = (*(fault_page_directory_entry))&(0x00000001);
  unsigned long page_valid_bit;
  unsigned long free_frame_number;
  unsigned long new_page_table;
  unsigned int i;
  // status bits for writing to the page directory and page table
  unsigned long valid_bit;
  unsigned long read_or_write;
  unsigned long user_or_kernel;
  unsigned long use_bit;
  unsigned long dirty_bit;
  // Test for legitemacy
  // Variables needed to handle the checking of address
  bool is_legitimate_val;
  VMPool* current_vm_pool = vm_pool_list_head;
  // check for validity of the input address, by traversin through the entire linked list
  // start off with the head of linked list
  while(1)
  {
    is_legitimate_val = current_vm_pool->is_legitimate(fault_virtual_address);
    if(is_legitimate_val == true)
    {
      break;
    }
    else
    {
      current_vm_pool = current_vm_pool->next_vm_pool;
      if(current_vm_pool == NULL)
      {
        Console::puts("An Illegal address was produced by the CPU");
        assert(false);
      }
    }
  }
  if(directory_valid_bit == 0)
  {
    // Memory access beyoung already allocated
    // Find a free frame pool to map the requested address
    //Console::puts("No page directory entry, so we need to create a new page table");Console::puts("\n");
    free_frame_number = process_mem_pool->get_frames(1);
    //Console::puts("Frame number got for alloting free space is ");Console::putui(free_frame_number);Console::puts(" \n");
    // Find a free frame to store the new page table
    new_page_table = process_mem_pool->get_frames(1);
      //Console::puts("Frame number got for new page table is ");Console::putui(new_page_table);Console::puts(" \n");
    // find the page table entry and make the first entry for newly created page table
    fault_page_table_offset = ((fault_virtual_address & 0x003FF000)>>12);
    //Console::puts("Page offset for faulty page is found to be ");Console::putui(fault_page_table_offset);Console::puts(" \n");
    //Console::puts("Page Directory offset for faulty page is found to be ");Console::putui(fault_page_directory_offset);Console::puts("\n");
    // Make the page directory entry first
    valid_bit = 0x00000001;
    read_or_write = 0x00000001;
    user_or_kernel = 0x00000001;
    use_bit = 0x00000000;
    dirty_bit = 0x00000000;
    *(fault_page_directory_entry) = (new_page_table*PAGE_SIZE)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
    //Console::puts("Page Directory entry made at ");Console::putui((unsigned long)(fault_page_directory_entry));Console::puts(" Content: ");Console::putui(*(fault_page_directory_entry));Console::puts("\n");
    // intialize the page table to all zeros
    for(i=0;i<1024;i++)
    {
      valid_bit = 0x00000000;
      read_or_write = 0x00000001;
      user_or_kernel = 0x00000001;
      use_bit = 0x00000000;
      dirty_bit = 0x00000000;
      // modified the allocation table
      *((unsigned long*)((1023<<22)|(fault_page_directory_offset<<12)|(i<<2)|0)) = (0<<20)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
    }
    valid_bit = 0x00000001;
    read_or_write = 0x00000001;
    user_or_kernel = 0x00000001;
    use_bit = 0x00000000;
    dirty_bit = 0x00000000;
    *((unsigned long*)((1023<<22)|(fault_page_directory_offset<<12)|(fault_page_table_offset<<2)|0)) = (free_frame_number*PAGE_SIZE)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
    //Console::puts("Page Table entry made at ");Console::putui((unsigned long)((new_page_table*PAGE_SIZE)+(4*fault_page_table_offset)));Console::puts(" Conent: ");Console::putui(*((unsigned long*)((new_page_table*PAGE_SIZE)+(4*fault_page_table_offset))));Console::puts("\n");
  }
  else
  {
    // page table already exists
    fault_page_table_offset = ((fault_virtual_address & 0x003FF000)>>12);
    //Console::puts("Page Table offset for faulty page is found to be ");Console::putui(fault_page_table_offset);Console::puts(" \n");
    fault_page_table_addr = (*(fault_page_directory_entry))&(0xFFFFF000);
    //Console::puts("Page Table entry for faulty page is found to be ");Console::putui((unsigned long)fault_page_table_addr);Console::puts(" \n");
    // check the valid bit on the page table entry
    page_valid_bit = *((unsigned long*)((1023<<22)|(fault_page_directory_offset<<12)|(fault_page_table_offset<<2)|0))&(0x00000001);
    if(page_valid_bit == 1)
    {
      Console::puts("Kernel and your page fault handling are not in good agreement\n");
      assert(false);
    }
    else
    {
      //Console::puts("Page Directory entry is present, But Page Table Entry is missing, Just creation of a new page is entry\n");
      // get a free frame from the processor memory pool
      free_frame_number = process_mem_pool->get_frames(1);
      // Put this frame number in the page table
      valid_bit = 0x00000001;
      read_or_write = 0x00000001;
      user_or_kernel = 0x00000001;
      use_bit = 0x00000000;
      dirty_bit = 0x00000000;
      *((unsigned long*)((1023<<22)|(fault_page_directory_offset<<12)|(fault_page_table_offset<<2)|0)) = (free_frame_number*PAGE_SIZE)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
    }

  }
  Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool* _vm_pool)
{
  // put the virtual pool in our linked-list pool
  PageTable::add_vm_pool_list(_vm_pool);
  Console::puts("registered VM pool\n");
}
void PageTable::add_vm_pool_list(VMPool* _vm_pool)
{
  // Buffer the inputs
  VMPool* vm_pool;
  vm_pool = _vm_pool;

  if(vm_pool_list_head == NULL)
  {
    // This means that the vm pool is not initialized
    // Make assignment to the head and make it the current node
    vm_pool_list_head = vm_pool;
    vm_pool_list_current = vm_pool_list_head;
  }
  else
  {
    // the list is already initiated
    // Make the assignment to the next node and make it the current node
    vm_pool_list_current->next_vm_pool = vm_pool;
    vm_pool_list_current = vm_pool_list_current->next_vm_pool;
    vm_pool_list_current->next_vm_pool = NULL;
  }
  Console::puts("A VM Pool is registered with starting address from ");Console::putui(vm_pool->base_address);Console::puts("\n");
}
void PageTable::free_page(unsigned long _page_no)
{
  // variables for changing the status of page
  unsigned long valid_bit;
  unsigned long read_or_write;
  unsigned long user_or_kernel;
  unsigned long use_bit;
  unsigned long dirty_bit;

  unsigned long page_directory_entry;
  unsigned long page_table_entry;
  page_directory_entry = (_page_no>>10);
  page_table_entry = (_page_no & 0x000003FF);

  valid_bit = 0x00000000;
  read_or_write = 0x00000001;
  user_or_kernel = 0x00000001;
  use_bit = 0x00000000;
  dirty_bit = 0x00000000;
  *((unsigned long*)((1023<<22)|(page_directory_entry << 12)|(page_table_entry << 2)|0)) = (0<<20)|(dirty_bit<<5)|(use_bit<<4)|(user_or_kernel<<2)|(read_or_write<<1)|(valid_bit);
  unsigned int cr3_read = read_cr3();
  write_cr3(cr3_read);
  Console::puts("Freed page with VPN of ");Console::putui(_page_no);Console::puts("\n");
}
