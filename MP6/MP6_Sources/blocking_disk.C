/*
     File        : blocking_disk.c

     Author      :
     Modified    :

     Description :

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
extern "C"
{
  #include "scheduler.H"
}
extern "C" Scheduler* SYSTEM_SCHEDULER;
extern  "C" BlockingDisk* SYSTEM_DISK;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
  : SimpleDisk(_disk_id, _size)
  {
    block_queue_id = SYSTEM_SCHEDULER->create_disk_blocked_queue();
    Console::puts("Obtained a block id queue of ");Console::putui(block_queue_id);Console::puts("\n");
  }

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf)
{
  SimpleDisk::read(_block_no, _buf);
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf)
{
  SimpleDisk::write(_block_no, _buf);
}

void BlockingDisk::wait_until_ready()
{
  Console::puts("Came here\n");
  while(!this->is_ready())
  {
    // Add this thread to blocking queue
    Console::puts("About to be added to blocked queue\n");
    SYSTEM_SCHEDULER->add_to_disk_blocked_queue(this->block_queue_id);
    // Yield the CPU
    Console::puts("About to yield the CPU to ready queue\n");
    SYSTEM_SCHEDULER->yield();
    Console::puts("Recovered back from the yield\n");
  }
  // Add this thread to Ready Queue
  SYSTEM_SCHEDULER->remove_from_disk_blocked_queue(this->block_queue_id);
  // yield the CPU
  SYSTEM_SCHEDULER->yield();
}

bool BlockingDisk::is_ready()
{

}
