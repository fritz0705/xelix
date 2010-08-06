#include <devices/pit/interface.h>
#include <interrupts/idt.h>
#include <interrupts/irq.h>
#include <devices/display/interface.h>

uint32 tick = 0;

static void timer_callback(registers_t regs)
{
   tick++;
   //log("Tick: ");
   //display_logDec(tick);
   //log("\n");
}

void pit_init(uint32 frequency)
{
   log("Initializing PIT at ");
   display_printDec(frequency);
   log("Hz.\n");
   // Firstly, register our timer callback.
   irq_registerHandler(IRQ0, &timer_callback);

   // The value we send to the PIT is the value to divide it's input clock
   // (1193180 Hz) by, to get our required frequency. Important to note is
   // that the divisor must be small enough to fit into 16-bits.
   uint32 divisor = 1193180 / frequency;

   // Send the command byte.
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   uint8 l = (uint8)(divisor & 0xFF);
   uint8 h = (uint8)( (divisor>>8) & 0xFF );

   // Send the frequency divisor.
   outb(0x40, l);
   outb(0x40, h);
}