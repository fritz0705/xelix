#pragma once

/* Copyright © 2011 Lukas Martini
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

#include <lib/generic.h>
#include <hw/cpu.h>

// Single linked list
typedef struct task {
	uint32_t pid;
	uint32_t parent;
	cpu_state_t* state;
	struct task* next;
	struct task* last;
} task_t;

int scheduler_state;

void scheduler_add();
void scheduler_terminateCurrentTask();
task_t* scheduler_getCurrentTask();
task_t* scheduler_select(cpu_state_t* lastRegs);
void scheduler_init();
