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

#include <stdio.h>
#include <raylib.h>
#include <skap_defines.h>

#define SKAP_IDX_IMAGE_MD_NAME_MAX_LEN 512

typedef struct {
  char name[SKAP_IDX_IMAGE_MD_NAME_MAX_LEN];
  u32 width;
  u32 height;
  u32 mipmaps;
  u8 format;
} skap_idx_image_md;

typedef struct {
  skap_idx_image_md metadata;
  usz blob_offset;
  usz blob_size;
} skap_idx_image;

void skap_idx_image_loadall(Image *imgs, const char **img_paths, usz size);

void skap_idx_image_unloadall(Image *imgs, usz size);

skap_idx_image skap_idx_image_create(const char *name, Image *img);

u8 skap_idx_image_append(FILE *fd, skap_idx_image *i);

void skap_idx_image_link_blob(skap_idx_image *i, usz blob_offset, usz blob_size);

u8 skap_idx_image_blob_append(FILE *fd, const char *name, Image *img);
