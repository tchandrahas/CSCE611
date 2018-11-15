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
  // Add this thread to blocking queue
  Console::puts("About to be added to blocked queue\n");
  SYSTEM_SCHEDULER->add_to_disk_blocked_queue(this->block_queue_id);
  Console::puts("The status of is_ready flag is ");Console::putui((unsigned int)this->is_ready());Console::puts("\n");
  if(!(this->is_ready()))
  {
    // Yield the CPU
    Console::puts("About to yield the CPU to ready queue\n");
    SYSTEM_SCHEDULER->yield();
    Console::puts("The status of is_ready flag is ");Console::putui((unsigned int)this->is_ready());Console::puts("\n");
  }
  Console::puts("Recovered back from the yield\n");
  Console::puts("The thread is removed from blocking queue\n");
  // Add this thread to Ready Queue
  SYSTEM_SCHEDULER->resume(SYSTEM_SCHEDULER->ExecutingThread);
  SYSTEM_SCHEDULER->DiskBlockingQueue[block_queue_id] = NULL;
  // yield the CPU
  Console::puts("Yielding the CPU again\n");
  SYSTEM_SCHEDULER->yield();
}

bool BlockingDisk::is_ready()
{

}
