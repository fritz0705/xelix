/* execve.c: Execve syscall
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

#include "execve.h"
#include <lib/log.h>
#include <tasks/scheduler.h>
#include <tasks/elf.h>

int sys_execve(struct syscall syscall)
{
	task_t* task = scheduler_getCurrentTask();
	log(LOG_DEBUG, "execve: Replacing [%s] with [%s]\n", task->name, syscall.params[0]);
	//void* entry = elf_load_file((void*)syscall.params[0]);
	//task->state->eip = entry;

	return 0;
}
