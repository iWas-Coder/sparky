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
#include <assert.h>
#include <sk_config.h>
#include <sk_client.h>
#include <sk_server.h>
#include <sk_defines.h>
#include <sk_renderer.h>

static void __draw_main_menu(void) {
  ClearBackground(BLACK);
  const char * const subtitle = "Press <ENTER> to start";
  u8 subtitle_size = 30;
  DrawText(SK_CLIENT_MAIN_MENU_TITLE,
           (GetScreenWidth() - MeasureText(SK_CLIENT_MAIN_MENU_TITLE,
                                           SK_CLIENT_MAIN_MENU_TITLE_SIZE)) / 2,
           (GetScreenHeight() - MeasureText(SK_CLIENT_MAIN_MENU_TITLE,
                                            SK_CLIENT_MAIN_MENU_TITLE_SIZE)) / 2,
           SK_CLIENT_MAIN_MENU_TITLE_SIZE,
           RAYWHITE);
  DrawText(subtitle,
           (GetScreenWidth() - MeasureText(subtitle, subtitle_size)) / 2,
           ((GetScreenHeight() - MeasureText(subtitle, subtitle_size)) / 2) + 115,
           subtitle_size,
           RAYWHITE);
  DrawText("v" sk_xstr(SK_VERSION),
           10,
           GetScreenHeight() - 25,
           20,
           RAYWHITE);
}

static inline void __draw_floor(void) {
  DrawPlane((Vector3) { 0, 0, 0 }, (Vector2) { 32, 32 }, DARKGRAY);
}

static inline void __draw_walls(void) {
  DrawCube((Vector3) { -16, 2.5f, 0 }, 1, 5, 32, MAROON);
  DrawCube((Vector3) { 16, 2.5f, 0 }, 1, 5, 32, LIME);
  DrawCube((Vector3) { 0, 2.5f, 16 }, 32, 5, 1, GOLD);
}

static void __draw_gameplay_world(sk_state *s) {
  ClearBackground(SKYBLUE);
  BeginMode3D(s->player.camera);
  sk_weapon_draw(&s->player.weapon, &s->player.camera, (Vector3) { 1, -1, -3 }, 0.5f);
  sk_player_draw(&s->player);
  __draw_floor();
  __draw_walls();
  EndMode3D();
}

static inline void __draw_crosshair(void) {
  DrawCircle(GetScreenWidth() / 2,
             GetScreenHeight() / 2,
             SK_CONFIG_CLIENT_CROSSHAIR_RADIUS,
             BLACK);
  DrawCircle(GetScreenWidth() / 2,
             GetScreenHeight() / 2,
             SK_CONFIG_CLIENT_CROSSHAIR_RADIUS - 0.9f,
             WHITE);
}

static void __draw_gameplay_hud(sk_state *s) {
  DrawText("v" sk_xstr(SK_VERSION),
           GetScreenWidth() - 100,
           GetScreenHeight() - 20,
           14,
           GRAY);
  DrawFPS(10, 10);
  DrawText(TextFormat("%.4f ms", GetFrameTime() * 1000), 10, 33, 20, LIME);
  if (s->is_online) {
    DrawText(TextFormat("N/A ms"), 10, 50, 20, LIME);
    DrawText(TextFormat("(d) N/A bps | (u) N/A bps"), 10, 70, 20, LIME);
  }
  __draw_crosshair();
  DrawText(TextFormat("%u", s->player.hp),
           100,
           GetScreenHeight() - 100,
           25,
           RAYWHITE);
  DrawText(TextFormat("%u | %u", s->player.weapon.ammo.magazine, s->player.weapon.ammo.reserve),
           GetScreenWidth() - 100,
           GetScreenHeight() - 100,
           25,
           RAYWHITE);
}

void sk_renderer_draw(sk_state *s) {
  BeginDrawing();
  switch (s->curr_scene.kind) {
  case SK_SCENE_KIND_MAIN_MENU:
    __draw_main_menu();
    break;
  case SK_SCENE_KIND_GAMEPLAY:
    __draw_gameplay_world(s);
    __draw_gameplay_hud(s);
    break;
  default:
    assert(0 && "Unreachable");
  }
  EndDrawing();
}
