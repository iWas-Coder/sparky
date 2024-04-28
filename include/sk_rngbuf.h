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


#pragma once

#include <sk_defines.h>

#define SK_RNGBUF_AT(rb, i) ((void *) ((u64) (rb)->data + ((((rb)->head + (i)) % (rb)->capacity) * (rb)->element_size)))

typedef struct {
  u32 capacity;
  usz element_size;
  u8 overwrite;
  u32 curr_len;
  i32 head;
  i32 tail;
  void *data;
} sk_rngbuf;

sk_rngbuf sk_rngbuf_create(u32 capacity, u32 element_size, u8 overwrite);

void sk_rngbuf_destroy(sk_rngbuf *rb);

u8 sk_rngbuf_push(sk_rngbuf *rb, void *value);

u8 sk_rngbuf_pop(sk_rngbuf *rb, void *out_value);

u8 sk_rngbuf_peek(const sk_rngbuf *rb, void *out_value);
