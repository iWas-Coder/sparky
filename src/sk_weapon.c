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


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sk_log.h>
#include <sk_weapon.h>
#include <raymath.h>

#define ICON_PATH_PLACEHOLDER         "assets/images/%s/icon.png"
#define MODEL_PATH_PLACEHOLDER        "assets/models/%s.glb"
#define SOUND_EQUIP_PATH_PLACEHOLDER  "assets/sounds/%s/equip.wav"
#define SOUND_SHOOT_PATH_PLACEHOLDER  "assets/sounds/%s/shoot.wav"
#define SOUND_RELOAD_PATH_PLACEHOLDER "assets/sounds/%s/reload.wav"

sk_weapon sk_weapon_create(sk_weapon_kind kind) {
  const char * const name = sk_weapon_kind2name[kind];
  const sk_weapon_ammo_spec ammo_spec = sk_weapon_kind2ammo[kind];
  sk_weapon w = {
    .kind = kind,
    .icon = LoadTexture(TextFormat(ICON_PATH_PLACEHOLDER, name)),
    .model = LoadModel(TextFormat(MODEL_PATH_PLACEHOLDER, name)),
    .model_anims = LoadModelAnimations(TextFormat(MODEL_PATH_PLACEHOLDER, name), (int *) &w.model_anims_count),
    .sound_equip = LoadSound(TextFormat(SOUND_EQUIP_PATH_PLACEHOLDER, name)),
    .sound_shoot = LoadSound(TextFormat(SOUND_SHOOT_PATH_PLACEHOLDER, name)),
    .sound_reload = LoadSound(TextFormat(SOUND_RELOAD_PATH_PLACEHOLDER, name)),
    .ammo = (sk_weapon_ammo_spec) {
      .magazine = ammo_spec.magazine,
      .reserve = ammo_spec.reserve * ammo_spec.magazine
    }
  };
  for (usz i = 0; i < w.model_anims_count; ++i) assert(IsModelAnimationValid(w.model, w.model_anims[i]));
  assert(IsSoundReady(w.sound_equip));
  assert(IsSoundReady(w.sound_shoot));
  assert(IsSoundReady(w.sound_reload));
  return w;
}

void sk_weapon_destroy(sk_weapon *w) {
  UnloadSound(w->sound_reload);
  UnloadSound(w->sound_shoot);
  UnloadSound(w->sound_equip);
  UnloadModelAnimations(w->model_anims, w->model_anims_count);
  UnloadModel(w->model);
  UnloadTexture(w->icon);
  *w = (sk_weapon) {0};
  w = 0;
}

void sk_weapon_draw(sk_weapon *w, Camera3D *cam) {
  Vector3 weapon_pos = Vector3Zero();
  Vector3 rotation_axis = Vector3Zero();
  f32 rotation_angle = 0;
  Vector3 scale = Vector3One();
  switch (w->kind) {
  case SK_WEAPON_KIND_7MM:
    weapon_pos = Vector3Add(cam->position, (Vector3) { 1, -1, -3 });
    rotation_axis = (Vector3) { 1, 0, 0 };
    rotation_angle = 275;
    scale = (Vector3) { 0.5f, 0.5f, 0.5f };
    break;
  case SK_WEAPON_KIND_AKM:
    weapon_pos = Vector3Add(cam->position, (Vector3) { 1, -1, -3 });
    rotation_axis = (Vector3) { 0, 1, 0 };
    rotation_angle = 90;
    scale = (Vector3) { 0.025f, 0.025f, 0.025f };
    break;
  default:
    assert(0 && "Unreachable");
  }
  DrawModelEx(w->model,
              weapon_pos,
              rotation_axis,
              rotation_angle,
              scale,
              WHITE);
}

u8 sk_weapon_shoot(sk_weapon *w, sk_uuid player_id, Camera3D *cam, sk_shot *shot) {
  if (!w->ammo.magazine || IsSoundPlaying(w->sound_reload)) return 0;
  ++w->model_anim_frame_count;
  UpdateModelAnimation(w->model, w->model_anims[0], w->model_anim_frame_count);
  if (w->model_anim_frame_count >= w->model_anims[0].frameCount) w->model_anim_frame_count = 0;
  PlaySound(w->sound_shoot);
  --w->ammo.magazine;
  shot->bullet = GetMouseRay(GetMousePosition(), *cam);
  shot->shooter_id = player_id;
  shot->weapon_kind = w->kind;
  SK_LOG_DEBUG("sk_shot = {");
  SK_LOG_DEBUG("  .bullet = {");
  SK_LOG_DEBUG("    .position = (%f, %f, %f),", shot->bullet.position.x, shot->bullet.position.y, shot->bullet.position.z);
  SK_LOG_DEBUG("    .direction = (%f, %f, %f)", shot->bullet.direction.x, shot->bullet.direction.y, shot->bullet.direction.z);
  SK_LOG_DEBUG("  },");
  SK_LOG_DEBUG("  .shooter_id = %s,", shot->shooter_id.value);
  SK_LOG_DEBUG("  .weapon_kind = %d", shot->weapon_kind);
  SK_LOG_DEBUG("};");
  return 1;
}

void sk_weapon_reload(sk_weapon *w) {
  const sk_weapon_ammo_spec ammo_spec = sk_weapon_kind2ammo[w->kind];
  if (!w->ammo.reserve || w->ammo.magazine == ammo_spec.magazine) return;
  PlaySound(w->sound_reload);
  u8 diff_ammo = w->ammo.reserve >= ammo_spec.magazine ? ammo_spec.magazine - w->ammo.magazine : w->ammo.reserve - w->ammo.magazine;
  w->ammo.magazine += diff_ammo;
  w->ammo.reserve -= diff_ammo;
}
