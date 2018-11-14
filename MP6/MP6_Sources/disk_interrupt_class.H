#ifndef _DISK_INTERRUPT_H_
#define _DISK_INTERRUPT_H_

#include "interrupts.H"
class disk_interrupt_class: public InterruptHandler {

public :

  disk_interrupt_class();
  /* Constructor  for disk interrupt handler */

  virtual void handle_interrupt(REGS *_r);
  /* Actual Interrupt Handler */

};

#endif
