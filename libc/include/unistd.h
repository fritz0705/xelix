#pragma once

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
#include <sys/types.h>

#define STDIN_FILENO 0;
#define STDOUT_FILENO 1;
#define STDERR_FILENO 2;

#define HOST_NAME_MAX 64

extern char** __environ;

int gethostname(char* name, size_t len);
int sethostname(const char* name, size_t len);
pid_t fork(void);

static inline uid_t getuid(void)
{
	return 0;
}

static inline uid_t geteuid(void)
{
	return 0;
}
