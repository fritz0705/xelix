#include <interrupts/interface.h>
#include <devices/display/interface.h>


// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
	interrupt_callback(regs);
}
