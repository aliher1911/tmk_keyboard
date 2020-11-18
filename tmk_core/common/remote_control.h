/*
Copyright 2011,2012,2013 Jun Wako <wakojun@gmail.com>
Copyrignt 2020 Oleg Afanasyev <oafanasiev@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Command format:
// 'KB', <cmd>, x, x, x, \n | \r

typedef enum {
  FRAME = 1,
  OVERFLOW = 2,
  BAD_COMMAND = 3,
  BAD_ARGUMENT = 4
} SerialCommandError;

// setup buffers etc for operation
void serial_control_init(void);
// process incoming/outgoing data
void serial_control_task(void);

#ifdef __cplusplus
}
#endif

#endif
