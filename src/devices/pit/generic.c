/* generic.c: Interface to the programmable interrupt timer
 * Copyright © 2010, 2011 Lukas Martini
 *
 * This file is part of Xelix.
 *
 * Xelix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Xelix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Xelix.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "interface.h"

#include <common/log.h>
#include <interrupts/interface.h>

uint64 tick = 0;
extern void switchcontext(); // in ASM

// The timer callback. Gets called every time the PIT fires.
static void timerCallback(registers_t regs)
{
	tick++;
}

// Initialize the PIT
void pit_init(uint16 frequency)
{
	log("pit: Setting frequqncy to %d Hz.\n", frequency);
	// Firstly, register our timer callback.
	interrupt_registerHandler(IRQ0, &timerCallback);

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

// Get the tick num
uint64 pit_getTickNum()
{
	return tick;
}
