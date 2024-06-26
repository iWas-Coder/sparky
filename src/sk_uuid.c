/*
 * GNU Sparky --- A 5v5 character-based libre tactical shooter
 * Copyright (C) 2024 Wasym A. Alonso
 *
 * This file is part of Sparky.
 *
 * Sparky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Sparky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Sparky.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <raylib.h>
#include <sk_uuid.h>
#include <sk_defines.h>

static const char chars[] = {
  '0', '1', '2', '3',
  '4', '5', '6', '7',
  '8', '9', 'a', 'b',
  'c', 'd', 'e', 'f'
};

sk_uuid sk_uuid_gen(void) {
  sk_uuid id = {0};
  for (u8 i = 0; i < SK_UUID_LEN; ++i) {
    if (i == 8 || i == 13 || i == 18 || i == 23) id.value[i] = '-';
    else id.value[i] = chars[GetRandomValue(0, 15)];
  }
  return id;
}
