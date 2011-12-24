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
#include <interrupts/interface.h>

#define MAX_CARDS 50

#define VENDOR_ID 0x8086
#define DEVICE_ID 0x100E

#define REG_CTL 0x00
#define REG_STATUS 0x8
#define REG_EECD 0x10
#define REG_EEPROM_READ 0x14
#define REG_VET 0x38

#define REG_INTR_CAUSE 0xc0
#define REG_INTR_MASK 0xd0
#define REG_INTR_MASK_CLR 0xd8

#define REG_RX_CTL 0x100
#define REG_TX_CTL 0x400

#define REG_RXDESC_ADDR_LO 0x2800
#define REG_RXDESC_ADDR_HI 0x2804
#define REG_RXDESC_LEN 0x2808
#define REG_RXDESC_HEAD 0x2810
#define REG_RXDESC_TAIL 0x2818

#define REG_RX_DELAY_TIMER 0x2820
#define REG_RADV 0x282c

#define REG_TXDESC_ADDR_LO 0x3800
#define REG_TXDESC_ADDR_HI 0x3804
#define REG_TXDESC_LEN 0x3808
#define REG_TXDESC_HEAD 0x3810
#define REG_TXDESC_TAIL 0x3818

#define REG_RECV_ADDR_LIST 0x5400

#define EERD_START (1 << 0)
#define EERD_DONE  (1 << 4)

#define E1000_EECD_SK 0x1
#define E1000_EECD_DI 0x4
#define E1000_EECD_DO 0x8

#define EEPROM_OFS_MAC 0x0

#define REG_TX_DELAY_TIMER 0x3820
#define REG_TADV 0x382c

#define CTL_RESET (1 << 26)
#define CTL_PHY_RESET (1 << 31)
#define CTL_AUTO_SPEED (1 << 5)
#define CTL_LINK_UP (1 << 6)

#define RCTL_ENABLE (1 << 1)
#define RCTL_BROADCAST (1 << 15)
#define RCTL_2K_BUFSIZE (0 << 16)

#define TCTL_ENABLE (1 << 1)
#define TCTL_PADDING (1 << 2)
#define TCTL_COLL_TSH (0x0f << 4)
#define TCTL_COLL_DIST (0x40 << 12)

#define RAH_VALID (1 << 31)

#define ICR_RECEIVE (1 << 7)

#define RX_BUFFER_NUM 8
#define TX_BUFFER_NUM 8

#define RX_BUFFER_SIZE 2048
#define TX_BUFFER_SIZE 2048

#define reg_in32(card, port) *((uint32_t*)(((char*)card->pciDevice->membase) + port))
#define reg_out32(card, port, value) (reg_in32(card, port) = value)

struct e1000_tx_descriptor {
	uint64_t buffer;
	uint16_t length;
	uint8_t checksum_offset;
	uint8_t cmd;
	uint8_t status;
	uint8_t checksum_start;
	uint16_t special;
} __attribute__((__packed__));

struct e1000_rx_descriptor {
	uint64_t buffer;
	uint16_t length;
	uint16_t padding;
	uint8_t status;
	uint8_t error;
	uint16_t padding2;
} __attribute__((__packed__));

struct e1000_card {
	pci_device_t* pciDevice;
	net_device_t* netDevice;

	struct e1000_tx_descriptor tx_desc[TX_BUFFER_NUM];
	uint8_t *tx_buffer;
	uint32_t tx_cur_buffer;

	struct e1000_rx_descriptor rx_desc[RX_BUFFER_NUM];
	uint8_t *rx_buffer;
	uint32_t rx_cur_buffer;
};

static int cards = 0;
static struct e1000_card e1000_cards[MAX_CARDS];

static inline void reg_out_flush(struct e1000_card *card)
{
	reg_out32(card, REG_STATUS, reg_in32(card, REG_STATUS));
}

/*
static void raiseEEPROMClock(struct e1000_card *card, uint32_t *eecd)
{
	*eecd |= E1000_EECD_SK;
	reg_out32(card, REG_EECD, *eecd);
	reg_out_flush(card);
}

static void lowerEEPROMClock(struct e1000_card *card, uint32_t *eecd)
{
	*eecd |= E1000_EECD_SK;
	reg_out32(card, REG_EECD, *eecd);
	reg_out_flush(card);
}

static uint16_t readEEPROMBits(struct e1000_card *card)
{
	uint32_t eecd = reg_in32(card, REG_EECD) & ~(E1000_EECD_DO | E1000_EECD_DI);
	uint16_t data = 0;;

	for (int i = 0; i < 16; ++i)
	{
		data <<= 1;
		raiseEEPROMClock(card, &eecd);

		eecd = reg_in32(card, REG_EECD) & ~E1000_EECD_DI;
		if (eecd & E1000_EECD_DO)
			data |= 1;

		lowerEEPROMClock(card, &eecd);
	}

	return data;
}
*/

static uint32_t readEERD(struct e1000_card *card, uint16_t offset)
{
	uint32_t eerd;
	reg_out32(card, REG_EEPROM_READ, (offset << 8) | EERD_START);

	for (int i = 0; i < 100; ++i)
	{
		eerd = reg_in32(card, REG_EEPROM_READ);
		if (eerd & EERD_DONE)
			break;
	}
	
	if (eerd & EERD_DONE)
		return (eerd >> 16) & 0xffff;

	return (uint32_t) -1;
}

static uint16_t readEEPROM(struct e1000_card *card, uint16_t offset)
{
	uint32_t data = readEERD(card, offset);
	if ((data == ((uint32_t) -1)))
	{
		log(LOG_WARN, "e1000: Could not read EEPROM via EERD\n");
		return -1;
	}

	return data;
}

static void loadMACAddress(struct e1000_card *card, uint16_t *ptr)
{
	for (int i = 0; i < 3; ++i)
		ptr[i] = readEEPROM(card, EEPROM_OFS_MAC + i);
}

static void handleInterrupt(cpu_state_t *state)
{
	log(LOG_DEBUG, "e1000: Got Interrupt\n");

	for (int i = 0; i < cards; ++i)
	{
		struct e1000_card *card = e1000_cards + i;

		uint32_t icr = reg_in32(card, REG_INTR_CAUSE);

		if (icr & ICR_RECEIVE)
		{
			uint32_t head = reg_in32(card, REG_RXDESC_HEAD);

			while (card->rx_cur_buffer != head)
			{
				size_t size = card->rx_desc[card->rx_cur_buffer].length;
				uint8_t status = card->rx_desc[card->rx_cur_buffer].status;

				if ((status & 0x1) == 0)
					break;

				size -= 4;

				card->rx_cur_buffer = (card->rx_cur_buffer + 1) % RX_BUFFER_NUM;
			}

			if (card->rx_cur_buffer == head)
				reg_out32(card, REG_RXDESC_TAIL, (head + RX_BUFFER_NUM - 1) % RX_BUFFER_NUM);
			else
				reg_out32(card, REG_RXDESC_TAIL, card->rx_cur_buffer);
		}
	}
}

static void enableCard(struct e1000_card *card)
{
	// Deactivate card
	reg_out32(card, REG_RX_CTL, 0);
	reg_out32(card, REG_TX_CTL, 0);

	// Reset card
	reg_out32(card, REG_CTL, CTL_PHY_RESET);
	reg_out32(card, REG_CTL, CTL_RESET);

	while (reg_in32(card, REG_CTL) & CTL_RESET);

	reg_out32(card, REG_CTL, CTL_AUTO_SPEED | CTL_LINK_UP);

	reg_out32(card, REG_RXDESC_ADDR_HI, 0);
	reg_out32(card, REG_RXDESC_ADDR_LO, (intptr_t)card->rx_desc);
	reg_out32(card, REG_RXDESC_LEN, RX_BUFFER_NUM * sizeof(struct e1000_rx_descriptor));
	reg_out32(card, REG_RXDESC_HEAD, 0);
	reg_out32(card, REG_RXDESC_TAIL, RX_BUFFER_NUM - 1);
	reg_out32(card, REG_RX_DELAY_TIMER, 0);
	reg_out32(card, REG_RADV, 0);

	reg_out32(card, REG_TXDESC_ADDR_HI, 0);
	reg_out32(card, REG_TXDESC_ADDR_LO, (intptr_t)card->tx_desc);
	reg_out32(card, REG_TXDESC_LEN, TX_BUFFER_NUM * sizeof(struct e1000_rx_descriptor));
	reg_out32(card, REG_TXDESC_HEAD, 0);
	reg_out32(card, REG_TXDESC_TAIL, 0);
	reg_out32(card, REG_TX_DELAY_TIMER, 0);
	reg_out32(card, REG_TADV, 0);

	reg_out32(card, REG_VET, 0);

	card->netDevice = kmalloc(sizeof(net_device_t));
	loadMACAddress(card, (uint16_t*)card->netDevice->hwaddr);

	uint64_t mac;
	loadMACAddress(card, (uint16_t*)&mac);
	reg_out32(card, REG_RECV_ADDR_LIST, mac & 0xffffffff);
	reg_out32(card, REG_RECV_ADDR_LIST + 4, (*(&mac + 2) & 0xFFFF) | RAH_VALID);

	card->rx_buffer = kmalloc(RX_BUFFER_NUM * RX_BUFFER_SIZE);
	card->tx_buffer = kmalloc(RX_BUFFER_NUM * RX_BUFFER_SIZE);

	for (int i = 0; i < RX_BUFFER_NUM; ++i)
	{
		card->rx_desc[i].length = RX_BUFFER_SIZE;
		card->rx_desc[i].buffer = (intptr_t)(card->rx_buffer + RX_BUFFER_SIZE * i);
	}

	card->tx_cur_buffer = 0;
	card->rx_cur_buffer = 0;

	reg_out32(card, REG_RX_CTL, RCTL_ENABLE | RCTL_BROADCAST | RCTL_2K_BUFSIZE);
	reg_out32(card, REG_TX_CTL, TCTL_ENABLE | TCTL_PADDING | TCTL_COLL_TSH | TCTL_COLL_DIST);

	strcpy(card->netDevice->name, "eth");
	strcpy(card->netDevice->name + 3, itoa(net_ether_offset++, 10));
	card->netDevice->mtu = 1500;
	card->netDevice->proto = NET_PROTO_ETH;
	card->netDevice->send = NULL;
	card->netDevice->data = card;

	net_register_device(card->netDevice);

	interrupts_registerHandler(card->pciDevice->interruptLine + IRQ0, handleInterrupt);
	reg_out32(card, REG_INTR_MASK_CLR, 0xFFFF);
	reg_out32(card, REG_INTR_MASK, 0xFFFF);
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
