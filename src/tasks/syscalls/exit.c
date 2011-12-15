/* exit.c: Exit syscall
 * Copyright © 2011 Fritz Grimpen
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

#include "exit.h"
#include <tasks/scheduler.h>
#include <lib/log.h>

int sys_exit(struct syscall syscall)
{
	task_t *task = scheduler_getCurrentTask()->parent;

	log(LOG_DEBUG, "Terminated task %d\n", task->pid);
	task->task_state = TASK_STATE_TERMINATED;
	scheduler_yield();
	return 0;
}
