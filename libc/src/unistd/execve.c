/* Copyright © 2011 Lukas Martini
 *
 * This file is part of Xlibc.
 *
 * Xlibc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Xlibc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Xlibc. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <unistd.h>

int execve(const char* filename, char* const argv[], char* const envp[])
{
	asm volatile("mov eax, 14;"
	             "mov ebx, %0;"
	             "mov ecx, %1;"
	             "int 0x80;"
	:: "r" (filename), "r" (argv) : "eax", "ebx", "ecx");
	return 0;
}