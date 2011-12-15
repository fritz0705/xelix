/* syscall.c: Syscalls
 * Copyright © 2011 Lukas Martini, Fritz Grimpen
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

#include "syscall.h"
#include <lib/generic.h>
#include <interrupts/interface.h>
#include <lib/log.h>
#include <tasks/scheduler.h>
#include <lib/print.h>

#include "syscalls.h"

static void syscallMain()
{
	task_t *task = scheduler_getCurrentTask();
	syscall_t call = syscall_table[task->syscall.num];

	int retval = call(task->syscall);

	task->parent->state->eax = retval;
	if (task->parent->task_state == TASK_STATE_BLOCKING)
		task->parent->task_state = TASK_STATE_RUNNING;
	task->task_state = TASK_STATE_TERMINATED;
	scheduler_yield();
}

static void intHandler(cpu_state_t* regs)
{
	task_t *task = scheduler_newSyscallTask(syscallMain, scheduler_getCurrentTask(), "SYSCALL");
	if (task->parent->sys_call_conv == TASK_SYSCONV_LINUX)
	{
		// Linux syscall calling convention
		task->syscall.num = regs->eax;
		task->syscall.params[0] = regs->ebx;
		task->syscall.params[1] = regs->ecx;
		task->syscall.params[2] = regs->edx;
		task->syscall.params[3] = regs->esi;
		task->syscall.params[4] = regs->edi;
		task->syscall.params[5] = (int)regs->ebp;
	}
	else
	{
		// Unix task->syscall calling convention
		task->syscall.num = regs->eax;
		task->syscall.params[0] = *((int *)regs->esp + sizeof(int));
		task->syscall.params[1] = *((int *)regs->esp + sizeof(int) * 2);
		task->syscall.params[2] = *((int *)regs->esp + sizeof(int) * 3);
		task->syscall.params[3] = *((int *)regs->esp + sizeof(int) * 4);
		task->syscall.params[4] = *((int *)regs->esp + sizeof(int) * 5);
		task->syscall.params[5] = *((int *)regs->esp + sizeof(int) * 6);
	}

	task->parent->task_state = TASK_STATE_BLOCKING;
	scheduler_add(task);
	scheduler_yield();
}

void syscall_init()
{
	interrupts_registerHandler(SYSCALL_INTERRUPT, intHandler);
}
