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
Thread* CurrentThread; // Variables that stores the current thread
Thread* NextThread; // variable that stores the next thread
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
  // Get the next thread and dispatch to it
  NextThread = CurrentThread->next_thread_fifo_queue;
  Console::puts("Thread with ThreadID ");Console::putui(CurrentThread->ThreadID());Console::puts(" yielded the CPU\n");Console::puts("\n");
  Conosle::puts("Now Dispatching to Thread with ThreadID  ");Console::putui(NextThread->ThreadID());Conosle::puts(" \n");
  CurrentThread.dispatch_to(NextThread);
}

void Scheduler::resume(Thread * _thread)
{
  add(_thread);
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
    ReadyQueue.next_thread_fifo_queue = NULL;
  }
  else
  {
    while(search_thread != NULL)
    {
      search_thread = search_thread->next_thread_fifo_queue;
    }
    search_thread->next_thread_fifo_queue = new_thread;
    new_thread->next_thread_fifo_queue = NULL;
  }
}

void Scheduler::terminate(Thread * _thread) {
  assert(false);
}
