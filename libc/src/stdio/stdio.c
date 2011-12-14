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
 
#include <stdio.h>

FILE _stdin = {
	.num = 0,
	.filename = "/dev/stdin",
	.offset = 0
};

FILE _stdout = {
	.num = 1,
	.filename = "/dev/stdout",
	.offset = 0
};

FILE _stderr = {
	.num = 2,
	.filename = "/dev/stderr",
	.offset = 0
};
