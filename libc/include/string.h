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

void memcpy(void* dest, void* src, uint32_t size);
void memset(void* ptr, uint8_t fill, uint32_t size);
size_t strlen(const char* str);
char* strtok(char* str, const char* delim);
char* strtok_r(char* s, const char* delim, char** last);
char* strcpy(char* dest, const char* src);
int strcmp(const char* str1, const char* str2);