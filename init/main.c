#include <common/generic.h>
#include <devices/display/interface.h>
#include <devices/cpu/interface.h>
#include <devices/keyboard/interface.h>
#include <memory/gdt.h>
#include <interrupts/idt.h>
#include <interrupts/pit.h>

void checkIntLenghts();
static void kbd_callback(registers_t regs);

void checkIntLenghts()
{
	print("Checking length of uint8... ");
	if(sizeof(uint8) == 1) print("Right\n");
	else panic("Got wrong lenght for uint8");
	
	print("Checking length of uint16... ");
	if(sizeof(uint16) == 2) print("Right\n");
	else panic("Got wrong lenght for uint16");
	
	print("Checking length of uint32... ");
	if(sizeof(uint32) == 4) print("Right\n");
	else panic("Got wrong lenght for uint32");
}

static void kbd_callback(registers_t regs)
{
	register int i;
	int scan;

	// tell keyboard we received the scancode
	scan = inb(0x60);
	i = inb(0x61);
	outb(0x61, i|0x80);
	outb(0x61, i);
	
	display_printHex(scan);
}

void kmain()
{
	
	display_init();
	
	display_setColor(0x0f);
	print("                                               \n");
	print("                                     decore    \n");
	print("                                               \n");
	display_setColor(0x07);
	
	
	
	print("Initialized Display.\n");
	checkIntLenghts();
	cpu_init();
	print("Initialized CPU\n");
	gdt_init();
	print("Initialized global descriptor table.\n");
	idt_init();
	print("Initialized interruptor descriptor table.\n");
	pit_init(50); //50Hz
	print("Initialized PIT\n");
	
	//keyboard_init();
	//print("Initialized Keyboard.\n");
	 
	 //outb(0x21,0xfd);
	 //outb(0xa1,0xff);
   //asm("sti");

	idt_registerHandler(IRQ1, &kbd_callback);

	
	//print("\nOhai! Welcome to Decore.\n\n");
	//asm volatile ("int $0x21");
	asm volatile ("int $0x3");
	//for(;;) asm volatile ("int $0x20");
}
