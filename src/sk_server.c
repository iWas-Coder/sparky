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
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sk_log.h>
#include <sk_state.h>
#include <sk_server.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <winsock2.h>
#include <synchapi.h>
#else
#include <time.h>
#endif

static volatile u8 running = 1;

static inline void handle_interrupt(i32 signum) {
  if (signum == SIGINT) running = 0;
}

static void wait_next_tick(double t) {
#if defined(_WIN32) || defined(_WIN64)
  Sleep(1000 * t);
#else
  long ns_t = 1e9 * t;
  struct timespec ts = {
    .tv_sec  = ns_t / (long) (1e9 - 1),
    .tv_nsec = ns_t % (long) (1e9 - 1)
  };
  nanosleep(&ts, &ts);
#endif
}

i32 sk_server_socket_create(void) {
  i32 sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_fd == -1) SK_LOG_ERROR("sk_server_socket_create :: %s", strerror(errno));
  else SK_LOG_INFO("UDP socket created");
  return sock_fd;
}

void sk_server_socket_destroy(i32 sock_fd) {
  if (sock_fd == -1) {
    SK_LOG_WARN("sk_server_socket_destroy :: `sock_fd` not valid");
    return;
  }
  close(sock_fd);
}

i32 sk_server_socket_bind(i32 sock_fd) {
  const struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(SK_SERVER_PORT),
    .sin_addr.s_addr = inet_addr("127.0.0.1")
  };
  i32 result = bind(sock_fd, (const struct sockaddr *) &addr, sizeof(addr));
  if (result == -1) {
    SK_LOG_ERROR("sk_server_socket_bind :: %s", strerror(errno));
    sk_server_socket_destroy(sock_fd);
  }
  else SK_LOG_INFO("UDP socket binded to 127.0.0.1:%u", SK_SERVER_PORT);
  return result;
}

u8 sk_server_run(void) {
  SK_LOG_INFO("Initializing %s", SK_SERVER_NAME);
  sk_state_global state_global = sk_state_global_create();
  i32 sock_fd = sk_server_socket_create();
  if (sock_fd == -1) return 1;
  if (sk_server_socket_bind(sock_fd) == -1) return 1;
  double dt = 1 / SK_SERVER_TICK_RATE;
  signal(SIGINT, handle_interrupt);
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char msg[SK_SERVER_MSG_MAX_SIZE];
  i32 msg_n = 0;
  while (running) {
    client_addr = (struct sockaddr_in) {0};
    client_addr_len = sizeof(client_addr);
    memset(msg, 0, sizeof(msg));
    msg_n = 0;
    msg_n = recvfrom(sock_fd,
                     msg,
                     sizeof(msg),
                     MSG_WAITALL,
                     (struct sockaddr *) &client_addr,
                     &client_addr_len);
    if (msg_n == -1) {
      SK_LOG_ERROR("recvfrom(2) :: %s", strerror(errno));
      continue;
    }
    msg[msg_n] = 0;
    if (!strcmp(msg, SK_SERVER_MSG_HELLO)) {
      SK_LOG_INFO("Connection from client (%s:%u) requested", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
      i8 assigned_lobby_id = -1;
      i8 assigned_lobby_slot_idx = -1;
      sk_state_global_assign_lobby(&state_global, &assigned_lobby_id, &assigned_lobby_slot_idx);
      if (sendto(sock_fd,
                 TextFormat(SK_SERVER_MSG_HOWDY, assigned_lobby_id, assigned_lobby_slot_idx),
                 strlen(TextFormat(SK_SERVER_MSG_HOWDY, assigned_lobby_id, assigned_lobby_slot_idx)),
                 0,
                 (struct sockaddr *) &client_addr,
                 client_addr_len) == -1) {
        SK_LOG_ERROR("sendto(2) :: %s", strerror(errno));
        SK_LOG_WARN("Connection from client (%s:%u) dropped", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        continue;
      }
      if (assigned_lobby_id == -1 || assigned_lobby_slot_idx == -1) {
        SK_LOG_INFO("Connection from client (%s:%u) rejected", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        continue;
      }
      SK_LOG_INFO("Connection from client (%s:%u) accepted", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
      // state_global.lobbies[assigned_lobby_id].players[assigned_lobby_slot_idx] = sk_player_create(assigned_lobby_id, assigned_lobby_slot_idx, SK_PLAYER_KIND_AGENT69);
    }
    wait_next_tick(dt);
  }
  sk_server_socket_destroy(sock_fd);
  SK_LOG_INFO("%s closed successfully", SK_SERVER_NAME);
  return 0;
}
