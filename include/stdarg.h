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
  

// No GCC or no variable argument counts – The choice is yours.
#ifdef __GNUC__
  typedef __builtin_va_list       va_list;
  #define va_start(ap, X)         __builtin_va_start(ap, X)
  #define va_arg(ap, type)        __builtin_va_arg(ap, type)
  #define va_end(ap)              __builtin_va_end(ap)
#endif
