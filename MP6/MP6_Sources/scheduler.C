/*
 File: scheduler.C

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

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler()
{
  // initialize the current executing thread to NULL
  ExecutingThread = NULL;
  num_disks_created = 0;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield()
{
  // Get the next thread and dispatch to it
  ExecutingThread = ReadyQueue;
  ReadyQueue = ReadyQueue->next_thread_fifo_queue;
  ExecutingThread->next_thread_fifo_queue = NULL;
  Console::puts("Thread dispatch happening to a thread with Id: ");Console::putui(ExecutingThread->ThreadId());Console::puts("\n");
  Console::puts("The ReadyQueue moved to thread with Id: ");Console::putui(ReadyQueue->ThreadId());Console::puts("\n");
  Thread::dispatch_to(ExecutingThread);
}

void Scheduler::resume(Thread * _thread)
{
  // buffer the input
  Thread* input_thread = _thread;
  Scheduler::add(input_thread);
}

void Scheduler::add(Thread * _thread)
{
  // Call the resume function
  Thread* input_thread = _thread;
  // Add the current to the back of queue
  // Add the Thread to the ready queue
  // By searching till the end of thread queue
  Thread* search_thread;
  search_thread = ReadyQueue;
  // First check if ready queue is filled or not
  // if not, the input thread will be the first to populate the queue
  if(ReadyQueue == NULL)
  {
    ReadyQueue = input_thread;
    ReadyQueue->next_thread_fifo_queue = NULL;
    Console::puts("Ready queue is found to be empty, So we add a thread with ID ");Console::putui(input_thread->ThreadId());Console::puts(" to the top of Ready queue\n");
  }
  else
  {
    while(search_thread->next_thread_fifo_queue != NULL)
    {
      Console::putui(search_thread->ThreadId());Console::puts(",");
      search_thread = search_thread->next_thread_fifo_queue;
    }
    Console::puts("\n");
    search_thread->next_thread_fifo_queue = input_thread;
    (search_thread->next_thread_fifo_queue)->next_thread_fifo_queue = NULL;
    input_thread->next_thread_fifo_queue = NULL;
    Console::puts("The next thread of Thread with ID ");Console::putui(search_thread->ThreadId());Console::puts(" is found to be empty, So we add thread with ID ");Console::putui((search_thread->next_thread_fifo_queue)->ThreadId());Console::puts(" next to this one\n");
  }

}

void Scheduler::terminate(Thread * _thread)
{
  // Buffer the inputs
  Thread* input_thread = _thread;
  Console::puts("Thread with Id ");Console::putui(input_thread->ThreadId());Console::puts(" is terminated\n");
  Scheduler::yield();
}

unsigned int Scheduler::create_disk_blocked_queue()
{
  num_disks_created++;
  DiskBlockingQueue[num_disks_created-1] = NULL;
  return (num_disks_created-1);
}

void Scheduler::add_to_disk_blocked_queue(unsigned int block_queue_id)
{
  Thread* search_thread;
  if(ExecutingThread->present_in_thread_block_queue == 1)
  {
    goto do_nothing;
  }
  search_thread = DiskBlockingQueue[block_queue_id];
  if(DiskBlockingQueue[block_queue_id] == NULL)
  {
    DiskBlockingQueue[block_queue_id] = ExecutingThread;
    ExecutingThread->present_in_thread_block_queue = 1;
    DiskBlockingQueue[block_queue_id]->next_thread_disk_block_queue = NULL;
    Console::puts("The Disk blocking queue is empty ");Console::puts("Thread with ID: ");Console::putui(ExecutingThread->ThreadId());Console::puts(" added to top of it\n");
  }

  else
  {
    while(search_thread->next_thread_disk_block_queue != NULL)
    {
      Console::putui(search_thread->ThreadId());Console::puts(",");
      search_thread = search_thread->next_thread_disk_block_queue;
    }
    Console::puts("\n");
    search_thread->next_thread_disk_block_queue = ExecutingThread;
    ExecutingThread->next_thread_disk_block_queue = NULL;
    Console::puts("The next thread of Thread with ID ");Console::putui(search_thread->ThreadId());Console::puts(" is found to be empty, So we add thread with ID ");Console::putui((search_thread->next_thread_fifo_queue)->ThreadId());Console::puts(" next to this one in blocking queue\n");
  }
  do_nothing:;
}

void Scheduler::remove_from_disk_blocked_queue(unsigned int block_queue_id)
{
  Thread* search_thread = DiskBlockingQueue[block_queue_id];
  Thread* previous_thread;
  while(search_thread != NULL)
  {
    if(search_thread == ExecutingThread)
    {
      previous_thread->next_thread_fifo_queue = search_thread->next_thread_fifo_queue;
      Scheduler::resume(search_thread);
      break;
    }

    else
    {
      previous_thread = search_thread;
      search_thread = search_thread->next_thread_disk_block_queue;
    }
  }
}
