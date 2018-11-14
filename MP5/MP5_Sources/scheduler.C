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
Thread* ReadyQueue; // pointer that points to the ready queue
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
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  // First get the current thread and make it unrunnable
  // Get the next thread and dispatch to it
  Thread* current_thread;
  current_thread = Thread::CurrentThread();
  Console::puts("Thread with ThreadID ");Console::putui(current_thread->ThreadId());Console::puts(" yielded the CPU\n");Console::puts("\n");
  Console::puts("Now Dispatching to Thread with ThreadID  ");Console::putui(ReadyQueue->ThreadId());Console::puts(" \n");
  Thread::dispatch_to(ReadyQueue);
}

void Scheduler::resume(Thread * _thread)
{
  // buffer the input
  Thread* input_thread = _thread;
  // If this thread is present in ready queue remove it from there
  if(ReadyQueue == input_thread)
  {
    // Remove this as head of ready queue
    Console::puts("ReadyQueue head to be changed from thread with Id ");Console::putui(ReadyQueue->ThreadId());Console::puts(" to thread with Id ");Console::putui((ReadyQueue->next_thread_fifo_queue)->ThreadId());Console::puts("\n");
    ReadyQueue = ReadyQueue->next_thread_fifo_queue;
  }
  // Add the current to the back of queue
  add(input_thread);
}

void Scheduler::add(Thread * _thread)
{
  // buffer the input first
  Thread* new_thread = _thread;
  // Add the Thread to the ready queue
  // By searching till the end of thread queue
  Thread* search_thread;
  search_thread = ReadyQueue;
  // First check if ready queue is filled or not
  // if not, the input thread will be the first to populate the queue
  if(ReadyQueue == NULL)
  {
    ReadyQueue = new_thread;
    ReadyQueue->next_thread_fifo_queue = NULL;
    Console::puts("Ready queue is found to be empty, So we add a thread with ID ");Console::putui(new_thread->ThreadId());Console::puts(" to the top of Ready queue\n");
  }
  else
  {
    while(search_thread->next_thread_fifo_queue != NULL)
    {
      search_thread = search_thread->next_thread_fifo_queue;
    }
    search_thread->next_thread_fifo_queue = new_thread;
    new_thread->next_thread_fifo_queue = NULL;
    Console::puts("The next thread of Thread with ID ");Console::putui(search_thread->ThreadId());Console::puts(" is found to be empty, So we add thread with ID ");Console::putui((search_thread->next_thread_fifo_queue)->ThreadId());Console::puts(" next to this one\n");
  }
}

void Scheduler::terminate(Thread * _thread)
{
  // Buffer the inputs
  Thread* input_thread = _thread;
  if(ReadyQueue == input_thread)
  {
    // Remove this as head of ready queue
    //Console::puts("ReadyQueue head to be changed from thread with Id ");Console::putui(ReadyQueue->ThreadId());Console::puts(" to thread with Id ");Console::putui((ReadyQueue->next_thread_fifo_queue)->ThreadId());Console::puts("\n");
    ReadyQueue = ReadyQueue->next_thread_fifo_queue;
  }
  //Console::puts("Thread with Id ");Console::putui(input_thread->ThreadId());Console::puts(" removed from ready queue\n");
  Thread::dispatch_to(ReadyQueue);
}
