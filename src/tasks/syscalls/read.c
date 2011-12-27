/* read.c: Read Syscall
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

#include "write.h"
#include <console/interface.h>
#include <lib/datetime.h>
#include <fs/vfs.h>

int sys_read(struct syscall syscall)
{
	if (syscall.params[0] == 0)
		return console_read(NULL, (char*)syscall.params[1], syscall.params[2]);

	vfs_file_t* fd = vfs_get_from_id(syscall.params[0]);
	if(fd == NULL)
		return -1;
	
	void* data = vfs_read(fd);
	memcpy((void*)syscall.params[1], data, syscall.params[2]);
	return 0;
}
