/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2022  Nikolaj Due Oesterbye <nikolaj@due-oesterbye.dk>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <errno.h>
#include <netinet/in.h>
#include "src/shared/bluetooth-definitions.h"
#include "src/shared/uuid.h"
#include "src/shared/util.h"

static uint128_t bluetooth_base_uuid = {
	.data = {	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
			0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB }
};

#define BASE_UUID16_OFFSET	2
#define BASE_UUID32_OFFSET	0

static void bt_uuid16_to_uuid128(const bt_uuid_t *src, bt_uuid_t *dst)
{
	uint16_t be16;

	dst->value.u128 = bluetooth_base_uuid;
	dst->type = BT_UUID128;

	/*
	 * No matter the system: 128-bit UUIDs should be stored
	 * as big-endian. 16-bit UUIDs are stored on host order.
	 */

	be16 = htons(src->value.u16);
	memcpy(&dst->value.u128.data[BASE_UUID16_OFFSET], &be16, sizeof(be16));
}

static void bt_uuid32_to_uuid128(const bt_uuid_t *src, bt_uuid_t *dst)
{
	uint32_t be32;

	dst->value.u128 = bluetooth_base_uuid;
	dst->type = BT_UUID128;

	/*
	 * No matter the system: 128-bit UUIDs should be stored
	 * as big-endian. 32-bit UUIDs are stored on host order.
	 */

	be32 = htonl(src->value.u32);
	memcpy(&dst->value.u128.data[BASE_UUID32_OFFSET], &be32, sizeof(be32));
}

void bt_uuid_to_uuid128(const bt_uuid_t *src, bt_uuid_t *dst)
{
	switch (src->type) {
	case BT_UUID128:
		*dst = *src;
		break;
	case BT_UUID32:
		bt_uuid32_to_uuid128(src, dst);
		break;
	case BT_UUID16:
		bt_uuid16_to_uuid128(src, dst);
		break;
	case BT_UUID_UNSPEC:
	default:
		break;
	}
}

int bt_uuid16_create(bt_uuid_t *btuuid, uint16_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = BT_UUID16;
	btuuid->value.u16 = value;

	return 0;
}

int bt_uuid32_create(bt_uuid_t *btuuid, uint32_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = BT_UUID32;
	btuuid->value.u32 = value;

	return 0;
}

int bt_uuid128_create(bt_uuid_t *btuuid, uint128_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = BT_UUID128;
	btuuid->value.u128 = value;

	return 0;
}
int bt_uuid_cmp(const bt_uuid_t *uuid1, const bt_uuid_t *uuid2)
{
	bt_uuid_t u1, u2;

	bt_uuid_to_uuid128(uuid1, &u1);
	bt_uuid_to_uuid128(uuid2, &u2);

	return memcmp(&u1.value.u128, &u2.value.u128, sizeof(uint128_t));
}

int bt_uuid_to_le(const bt_uuid_t *src, void *dst)
{
	bt_uuid_t uuid;

	switch (src->type) {
	case BT_UUID16:
		put_le16(src->value.u16, dst);
		return 0;
	case BT_UUID32:
		bt_uuid32_to_uuid128(src, &uuid);
		src = &uuid;
		/* Fallthrough */
	case BT_UUID128:
		/* Convert from 128-bit BE to LE */
		bswap_128(&src->value.u128, dst);
		return 0;
	case BT_UUID_UNSPEC:
	default:
		return -EINVAL;
	}
}
