#include "disk_interrupt_class.H"
#include "thread.H"
#include "console.H"
extern "C"
{
  #include "scheduler.H"
}
extern "C"
{
  #include "blocking_disk.H"
}
extern "C" Scheduler* SYSTEM_SCHEDULER;
extern  "C" BlockingDisk* SYSTEM_DISK;
disk_interrupt_class::disk_interrupt_class()
{
  // Basically do nothing for constructing the class
}

void disk_interrupt_class::handle_interrupt(REGS *_r)
{
  Console::puts("Entered the interrupt handling\n");
  unsigned int block_queue_id = SYSTEM_DISK->block_queue_id;
  Thread* traverse_thread = SYSTEM_SCHEDULER->DiskBlockingQueue[block_queue_id];
  while(traverse_thread != NULL)
  {
    Console::puts("Traversed to thread with Id: ");Console::putui(traverse_thread->ThreadId());Console::puts(" in the block queue\n");
    Thread::dispatch_to(traverse_thread);
    traverse_thread = traverse_thread->next_thread_disk_block_queue;

  }
}
