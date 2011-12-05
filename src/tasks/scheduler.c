/* scheduler.c: Selecting which task is being executed next
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

#include "scheduler.h"

#include <lib/log.h>
#include <memory/kmalloc.h>
#include <hw/cpu.h>
#include <interrupts/interface.h>
#include <lib/panic.h>
#include <memory/vm.h>

#define STACKSIZE PAGE_SIZE
#define STATE_OFF 0
#define STATE_INITIALIZING 1
#define STATE_INITIALIZED 2

task_t* currentTask = NULL;
uint64_t highestPid = -1;

/* If we kill a process using scheduler_terminateCurrentTask, we also
 * fire an IRQ to switch to the next process.  However, that way, the
 * next process running would get less cpu time, as the next timer
 * interrupt happens to be faster. Therefore, if this var is set, the
 * scheduler 'skips' one tick, effectively giving the running process
 * more time.
 */
#define SKIP_WAIT 2
#define SKIP_NEXT 1
#define SKIP_OFF  0
int skipnext = SKIP_OFF;

void scheduler_terminateCurrentTask()
{
	log(LOG_DEBUG, "scheduler: Deleting current task\n");

	if(currentTask->next == currentTask)
		currentTask = NULL;
	else
	{
		currentTask->next->last = currentTask->last;
		currentTask->last->next = currentTask->next;
	}

	skipnext = SKIP_WAIT;
	while(true) asm("int 0x20; hlt");
}

void scheduler_terminateKernelTask()
{
	scheduler_terminateCurrentTask();
}

void scheduler_remove(task_t *t)
{
	log(LOG_DEBUG, "scheduler: Deleting task %d\n", t->pid);

	t->next->last = t->last;
	t->last->next = t->next;
}

static struct vm_context *setupMemoryContext(void *stack)
{
	log(LOG_DEBUG, "scheduler: Setup new Memory Context [%d]\n", stack);
	struct vm_context *ctx = vm_new();
	
	/* Map the interrupt-handler stack */
	struct vm_page *intStack = vm_new_page();
	intStack->allocated = 1;
	intStack->section = VM_SECTION_KERNEL;
	intStack->virt_addr = NULL;
	intStack->phys_addr = NULL;

	vm_add_page(ctx, intStack);

	/* Protect unused kernel space (0x7fff0000 - 0x7fffc000) */
	for(int addr = 0x7fff0000; addr <= 0x7fffc000; addr += PAGE_SIZE)
	{
		struct vm_page *currPage = vm_new_page();
		currPage->allocated = 0;
		currPage->section = VM_SECTION_KERNEL;
		currPage->readonly = 1;
		currPage->virt_addr = (void *)addr;

		vm_add_page(ctx, currPage);
	}

	struct vm_page *stackPage = vm_new_page();
	stackPage->allocated = 1;
	stackPage->section = VM_SECTION_STACK;
	stackPage->virt_addr = (void *)0x7fffe000;
	stackPage->phys_addr = stack;

	/* Additional stack space if already allocated stack is full */

	struct vm_page *stackPage2 = vm_new_page();
	stackPage2->section = VM_SECTION_STACK;
	stackPage2->virt_addr = (void *)0x7fffd000;

	vm_add_page(ctx, stackPage);
	vm_add_page(ctx, stackPage2);

	/* Map memory from 0x100000 to 0x17f000 (Kernel-related data) */
	int pos = 0x100000;
	while (pos <= 0x17f000)
	{
		struct vm_page *currPage = vm_new_page();
		currPage->allocated = 1;
		currPage->section = VM_SECTION_KERNEL;
		currPage->readonly = 1;
		currPage->virt_addr = (void *)pos;
		currPage->phys_addr = (void *)pos;

		vm_add_page(ctx, currPage);

		pos += PAGE_SIZE;
	}

	/* Map unused interrupt handler */

	struct vm_page *intHandler = vm_new_page();
	intHandler->section = VM_SECTION_KERNEL;
	intHandler->readonly = 1;
	intHandler->allocated = 1;
	intHandler->virt_addr = (void *)0x7ffff000;
	intHandler->phys_addr = (void *)0;

	vm_add_page(ctx, intHandler);

	return ctx;
}

task_t *scheduler_newTask(task_t *parent)
{
	task_t *thisTask = kmalloc(sizeof(task_t));

	thisTask->pid = ++highestPid;
	thisTask->parent = parent;
	thisTask->task_state = TASK_STATE_RUNNING;
	thisTask->sys_call_conv = (parent == NULL) ? TASK_SYSCONV_LINUX : parent->sys_call_conv;

	return thisTask;
}

task_t *scheduler_newKernelTask(void *entry, task_t *parent)
{
	task_t *thisTask = scheduler_newTask(parent);
	
	thisTask->memory_context = vm_kernelContext;
	thisTask->type = TASK_TYPE_KERNEL;
	
	void *stack = kmalloc_a(STACKSIZE);
	memset(stack, 0, STACKSIZE);

	thisTask->state = stack + STACKSIZE - sizeof(cpu_state_t) - 3;
	thisTask->state->esp = stack + STACKSIZE - 3;
	thisTask->state->ebp = thisTask->state->esp;

	*(thisTask->state->ebp + 1) = (intptr_t)scheduler_terminateKernelTask;
	*(thisTask->state->ebp + 2) = NULL;

	thisTask->state->eip = entry;
	thisTask->state->eflags = 0x200;
	thisTask->state->cs = 0x08;
	thisTask->state->ds = 0x10;
	thisTask->state->ss = 0x10;

	return thisTask;
}

task_t *scheduler_newUserTask(void *entry, task_t *parent)
{
	task_t *thisTask = scheduler_newTask(parent);

	void *stack = kmalloc_a(STACKSIZE);
	memset(stack, 0, STACKSIZE);
	
	thisTask->memory_context = setupMemoryContext(stack);
	thisTask->state = stack + STACKSIZE - sizeof(cpu_state_t) - 3;
	thisTask->type = TASK_TYPE_USER;

	thisTask->state->esp = (char *)0x7ffff000 - sizeof(cpu_state_t) - 3;
	thisTask->state->ebp = thisTask->state->esp;

	*((char *)stack + STACKSIZE - 3 + 1) = (intptr_t)scheduler_terminateCurrentTask;
	*((char *)stack + STACKSIZE - 3 + 2) = NULL;

	thisTask->state->eip = entry;
	thisTask->state->eflags = 0x200;
	thisTask->state->cs = 0x08;
	thisTask->state->ds = 0x10;
	thisTask->state->ss = 0x10;

	return thisTask;
}

task_t *scheduler_newSyscallTask(void *entry, task_t *parent)
{
	return scheduler_newKernelTask(entry, parent);
}

// Add new task to schedule.
void scheduler_add(task_t *task)
{
	interrupts_disable();

	// No task yet?
	if(currentTask == NULL)
	{
		currentTask = task;
		task->next = task;
		task->last = task;
	} else {
		task->next = currentTask->next;
		task->last = currentTask;
		currentTask->next = task;
	}

	interrupts_enable();
	
	log(LOG_INFO, "scheduler: Registered new task with PID %d\n", task->pid);
}

task_t* scheduler_getCurrentTask()
{
	return currentTask;
}

// Called by the PIT a few hundred times per second.
task_t* scheduler_select(cpu_state_t* lastRegs)
{
	if(unlikely(scheduler_state == STATE_INITIALIZING))
	{
		scheduler_state = STATE_INITIALIZED;
		return currentTask;
	}

	currentTask->state = lastRegs;

	if(skipnext == SKIP_WAIT) skipnext = SKIP_NEXT;
	else if(skipnext == SKIP_NEXT)
	{
		skipnext = SKIP_OFF;
		return currentTask;
	}

	while (1)
	{
		currentTask = currentTask->next;
		
		if (currentTask->task_state == TASK_STATE_KILLED ||
				currentTask->task_state == TASK_STATE_TERMINATED)
		{
			if (currentTask->next == currentTask)
				currentTask->next = NULL;
			scheduler_remove(currentTask);
		}

		if (unlikely(currentTask == NULL || currentTask->task_state == TASK_STATE_RUNNING))
			break;
	}

	return currentTask;
}

void scheduler_init()
{
	scheduler_state = STATE_INITIALIZING;
}
