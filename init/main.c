#include <common/generic.h>
#include <devices/display/interface.h>
#include <devices/cpu/interface.h>
#include <devices/keyboard/interface.h>
#include <memory/gdt.h>
#include <interrupts/idt.h>
#include <interrupts/irq.h>
#include <devices/pit/interface.h>

void checkIntLenghts();
static void kbd_callback(registers_t regs);

void checkIntLenghts()
{
	log("Checking length of uint8... ");
	if(sizeof(uint8) == 1) log("Right\n");
	else panic("Got wrong lenght for uint8");
	
	log("Checking length of uint16... ");
	if(sizeof(uint16) == 2) log("Right\n");
	else panic("Got wrong lenght for uint16");
	
	log("Checking length of uint32... ");
	if(sizeof(uint32) == 4) log("Right\n");
	else panic("Got wrong lenght for uint32");
}

void kmain()
{
	
	display_init();
	
	display_setColor(0x0f);
	print("                                               \n");
	print("                                     decore    \n");
	print("                                               \n");
	display_setColor(0x07);
	
	
	
	log("Initialized Display.\n");
	checkIntLenghts();
	cpu_init();
	log("Initialized CPU\n");
	gdt_init();
	log("Initialized global descriptor table.\n");
	idt_init();
	log("Initialized interruptor descriptor table.\n");
	pit_init(50); //50Hz
	log("Initialized PIT\n");
	
	keyboard_init();
	log("Initialized Keyboard.\n");
	
	
	
	
	log("\nDecore is up.\n\n");
	
	while(1)
	{
		
	}
}
