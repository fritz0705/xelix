/* ne2k.c: Driver for network cards that are compliant to the Ne2000
 * reference design.
 * Copyright © 2011 Lukas Martini
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
 * along with Xelix. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ne2k.h"

#include <hw/rtl8139.h>
#include <lib/log.h>
#include <lib/portio.h>
#include <interrupts/interface.h>
#include <memory/kmalloc.h>

#define VENDOR_ID 0x10ec
#define DEVICE_ID 0x8029

// This driver supports only that many cards
#define MAX_CARDS 50 

#define NE_CMD	  0
#define NE_PSTART   1
#define NE_PSTOP	2
#define NE_BNDRY	3

#define NE_TSR	  4 // R
#define NE_TPSR	 4 // W

#define NE_TBCR0	5
#define NE_TBCR1	6

#define NE_ISR	  7

#define NE_RSAR0	8
#define NE_RSAR1	9
#define NE_RBCR0	10
#define NE_RBCR1	11

#define NE_RCR	  12
#define NE_TCR	  13
#define NE_DCR	  14
#define NE_IMR	  15

// Register page 1
#define NE_PAR	  1 // PAR0..5
#define NE_CURR	 7
#define NE_MAR	  8

// Packet ring buffer offsets
#define PAGE_TX	 0x40
#define PAGE_RX	 0x50
#define PAGE_STOP   0x80

#define NE_RESET	0x1F
#define NE_DATA	 0x10

#define RX_BUFFER_SIZE 0x2000
#define TX_BUFFER_SIZE 0x1000

struct ne2k_card {
	pci_device_t *device;
	char macAddr[6];
	void* phys;
	uint8_t nextPacket;
	bool txInProgress;
};

static struct ne2k_card ne2k_cards[MAX_CARDS];

static void interruptHandler(cpu_state_t* regs)
{
	log(LOG_DEBUG, "Hooray! Interrupt!\n");
}

static void enableCard(struct ne2k_card *card)
{
	// Register interrupt handler
	interrupts_registerHandler(card->device->interruptLine + IRQ0, interruptHandler);
	
	// Reset card
	portio_out8(card->device->iobase + NE_CMD, 0x21);
	
	/* Enable 16-bit transfer, turn on monitor mode to avoid receiving
	 * packets and turn on loopback.
	 */
	portio_out8(card->device->iobase + NE_DCR, 0x09);
	portio_out8(card->device->iobase + NE_RCR, 0x20);
	portio_out8(card->device->iobase + NE_TCR, 0x02);

	// Disable card interrupts
	portio_out8(card->device->iobase + NE_ISR, 0xFF);
	portio_out8(card->device->iobase + NE_IMR, 0);

	// Read the MAC address from PROM
	portio_out8(card->device->iobase + NE_RSAR0, 0);
	portio_out8(card->device->iobase + NE_RSAR1, 0);
	portio_out8(card->device->iobase + NE_RBCR0, 32);
	portio_out8(card->device->iobase + NE_RBCR1, 0);
	portio_out8(card->device->iobase + NE_CMD, 0x0A);

	for(int i = 0; i < 6; i++)
		card->macAddr[i] = portio_in16(card->device->iobase + NE_DATA) & 0xff;

	// Setup the packet ring buffer
	portio_out8(card->device->iobase + NE_CMD, 0x61);
	card->nextPacket = PAGE_RX + 1;
	portio_out8(card->device->iobase + NE_CURR, card->nextPacket);

	portio_out8(card->device->iobase + NE_CMD, 0x21);

	portio_out8(card->device->iobase + NE_PSTART, PAGE_RX);
	portio_out8(card->device->iobase + NE_BNDRY, PAGE_RX);
	portio_out8(card->device->iobase + NE_PSTOP, PAGE_STOP);

	// Accept broadcast and runt packets
	portio_out8(card->device->iobase + NE_RCR, 0x06);
	portio_out8(card->device->iobase + NE_TCR, 0);

	// Clear pending interrupts, enable them all, and begin card operation
	portio_out8(card->device->iobase + NE_ISR, 0xFF);
	portio_out8(card->device->iobase + NE_IMR, 0x3F);
	portio_out8(card->device->iobase + NE_CMD, 0x22);
}

void ne2k_init()
{
	log_setLogLevel(LOG_DEBUG);
	memset(ne2k_cards, 0, MAX_CARDS * sizeof(struct ne2k_card));

	pci_device_t** devices = (pci_device_t**)kmalloc(sizeof(void*) * MAX_CARDS);
	uint32_t numDevices = pci_searchDevice(devices, VENDOR_ID, DEVICE_ID, MAX_CARDS);
	
	log(LOG_INFO, "ne2k: Discovered %d device%p.\n", numDevices);
	
	int i;
	for(i = 0; i < numDevices; i++)
	{
		ne2k_cards[i].device = devices[i];
		enableCard(&ne2k_cards[i]);

		log(LOG_INFO, "ne2k: %d:%d.%d, iobase 0x%x, interrupt %d, MAC Address %x:%x:%x:%x:%x:%x\n",
				devices[i]->bus,
				devices[i]->dev,
				devices[i]->func,
				devices[i]->iobase,
				devices[i]->interruptLine + IRQ0,
				ne2k_cards[i].macAddr[0],
				ne2k_cards[i].macAddr[1],
				ne2k_cards[i].macAddr[2],
				ne2k_cards[i].macAddr[3],
				ne2k_cards[i].macAddr[4],
				ne2k_cards[i].macAddr[5]
			 );
	}
}
