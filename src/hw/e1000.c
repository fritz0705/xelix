/* e1000.c: Driver for the Intel PRO/1000 network interface card
 * Copyright © <year> <your name>
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

#include "e1000.h"
#include <hw/pci.h>

#include <memory/kmalloc.h>
#include <lib/log.h>
#include <net/net.h>

#define MAX_CARDS 50

#define VENDOR_ID 0x8086
#define DEVICE_ID 0x100E

struct e1000_card {
	pci_device_t* pciDevice;
	net_device_t* netDevice;
};

static int cards = 0;
static struct e1000_card e1000_cards[MAX_CARDS];

void e1000_init()
{
	memset(e1000_cards, 0, sizeof(struct e1000_card) * MAX_CARDS);
	pci_device_t** devices = (pci_device_t**)kmalloc(sizeof(void*) * MAX_CARDS);
	uint32_t num_devices = pci_searchDevice(devices, VENDOR_ID, DEVICE_ID, MAX_CARDS);

	log(LOG_INFO, "e1000: Discovered %d device%p.\n", num_devices);

	for (int i = 0; i < num_devices; ++i, ++cards)
	{
		e1000_cards[i].pciDevice = devices[i];
	}
}
