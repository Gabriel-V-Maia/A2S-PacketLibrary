#ifndef A2S_H
#define A2S_H

#include <stdint.h>

#define A2S_INFO_RESPONSE   0x49  /* 'I' */
#define A2S_PLAYER_RESPONSE 0x44  /* 'D' */
#define A2S_CHALLENGE       0x41  /* 'A' */

#define A2S_MAX_PLAYERS 64
#define A2S_BUF_SIZE    4096

typedef struct {
    char     name[256];
    char     map[256];
    char     game[256];
    uint8_t  players;
    uint8_t  max_players;
    uint8_t  bots;
    uint8_t  vac;
    uint8_t  protocol;
} A2SInfo;

typedef struct {
    uint8_t  index;
    char     name[256];
    int32_t  score;
    float    duration;
} A2SPlayer;

int a2s_info(const char *ip, uint16_t port, A2SInfo *out);
int a2s_players(const char *ip, uint16_t port, A2SPlayer *out, int max);

#endif
