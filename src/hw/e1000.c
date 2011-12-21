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
#include <net/ether.h>
#include <lib/string.h>

#define MAX_CARDS 50

#define VENDOR_ID 0x8086
#define DEVICE_ID 0x100E

#define REG_CTL 0x00
#define REG_RX_CTL 0x100
#define REG_TX_CTL 0x400

#define CTL_RESET (1 << 26)
#define CTL_PHY_RESET (1 << 31)

#define reg_in32(card, port) *((uint32_t*)(((char*)card->pciDevice->membase) + port))
#define reg_out32(card, port, value) (reg_in32(card, port) = value)

struct e1000_card {
	pci_device_t* pciDevice;
	net_device_t* netDevice;
};

static int cards = 0;
static struct e1000_card e1000_cards[MAX_CARDS];

static void enableCard(struct e1000_card *card)
{
	// Deactivate card
	reg_out32(card, REG_RX_CTL, 0);
	reg_out32(card, REG_TX_CTL, 0);

	// Reset card
	reg_out32(card, REG_CTL, CTL_PHY_RESET);

	card->netDevice = kmalloc(sizeof(net_device_t));
	strcpy(card->netDevice->name, "eth");
	strcpy(card->netDevice->name + 3, itoa(net_ether_offset++, 10));
	memset(card->netDevice->hwaddr, 0, 6);
	card->netDevice->mtu = 1500;
	card->netDevice->proto = NET_PROTO_ETH;
	card->netDevice->send = NULL;
	card->netDevice->data = card;

	net_register_device(card->netDevice);
}

void e1000_init()
{
	memset(e1000_cards, 0, sizeof(struct e1000_card) * MAX_CARDS);
	pci_device_t** devices = (pci_device_t**)kmalloc(sizeof(void*) * MAX_CARDS);
	uint32_t num_devices = pci_searchDevice(devices, VENDOR_ID, DEVICE_ID, MAX_CARDS);

	log(LOG_INFO, "e1000: Discovered %d device%p.\n", num_devices);

	for (int i = 0; i < num_devices; ++i, ++cards)
	{
		struct e1000_card *card = &e1000_cards[i];

		card->pciDevice = devices[i];
		enableCard(card);
	}
}
