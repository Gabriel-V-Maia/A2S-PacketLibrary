#include <stdio.h>
#include "a2s.h"

int main(void)
{
    const char *ip   = "192.168.1.100";
    uint16_t    port = 27015;
  
    A2SInfo info;
    if (a2s_info(ip, port, &info) == 0) 
    {
        printf("=== A2S_INFO ===\n");
        printf("Servidor : %s\n", info.name);
        printf("Mapa     : %s\n", info.map);
        printf("Jogo     : %s\n", info.game);
        printf("Players  : %d/%d  (bots: %d)\n",
               info.players, info.max_players, info.bots);
        printf("VAC      : %s\n", info.vac ? "sim" : "não");
    } else 
    {
        fprintf(stderr, "a2s_info falhou\n");
    }

    A2SPlayer players[A2S_MAX_PLAYERS];
    int count = a2s_players(ip, port, players, A2S_MAX_PLAYERS);
    if (count > 0)
    {
        printf("\n=== A2S_PLAYERS (%d) ===\n", count);
        for (int i = 0; i < count; i++) {
            printf("[%2d] %-24s  score=%-5d  tempo=%.0fs\n",
                   i + 1,
                   players[i].name,
                   players[i].score,
                   players[i].duration);
        }
    } else {
        fprintf(stderr, "a2s_players falhou ou servidor vazio\n");
    }

    return 0;
}
