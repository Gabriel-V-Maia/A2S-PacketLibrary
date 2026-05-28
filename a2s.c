#include "a2s.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>  

static uint8_t read_byte(const uint8_t *buf, int *off)
{
    return buf[(*off)++];
}

static int16_t read_short(const uint8_t *buf, int *off)
{
    int16_t v;
    memcpy(&v, buf + *off, 2);
    *off += 2;
    return v;
}

static int32_t read_long(const uint8_t *buf, int *off)
{
    int32_t v;
    memcpy(&v, buf + *off, 4);
    *off += 4;
    return v;
}

static float read_float(const uint8_t *buf, int *off)
{
    float v;
    memcpy(&v, buf + *off, 4);
    *off += 4;
    return v;
}

static void read_string(const uint8_t *buf, int *off, char *out, int max)
{
    int i = 0;
    while (buf[*off] != '\0' && i < max - 1)
        out[i++] = buf[(*off)++];
    out[i] = '\0';
    (*off)++;  
}

static int make_socket(int timeout_ms)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) return -1;

    struct timeval tv;
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    return fd;
}

static int send_recv(int fd, struct sockaddr_in *addr,
                     const uint8_t *req, int req_len,
                     uint8_t *buf, int buf_max)
{
    sendto(fd, req, req_len, 0, (struct sockaddr *)addr, sizeof(*addr));
    return recvfrom(fd, buf, buf_max, 0, NULL, NULL);
}



int a2s_info(const char *ip, uint16_t port, A2SInfo *out)
{
    int fd = make_socket(3000);
    if (fd < 0) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    const uint8_t req[] =
        "\xFF\xFF\xFF\xFF\x54Source Engine Query\x00";

    uint8_t buf[A2S_BUF_SIZE];
    int n = send_recv(fd, &addr, req, sizeof(req) - 1, buf, sizeof(buf));
    close(fd);

    if (n < 5) return -1;

    int off = 0;
    read_long(buf, &off);               
    uint8_t tipo = read_byte(buf, &off);

    if (tipo != A2S_INFO_RESPONSE) return -1;

    out->protocol    = read_byte(buf, &off);
    read_string(buf, &off, out->name,  256);
    read_string(buf, &off, out->map,   256);
    read_string(buf, &off, out->game,  256);
    read_short(buf, &off);            
    out->players     = read_byte(buf, &off);
    out->max_players = read_byte(buf, &off);
    out->bots        = read_byte(buf, &off);
    read_byte(buf, &off);               
    read_byte(buf, &off);             
    read_byte(buf, &off);               
    out->vac         = read_byte(buf, &off);

    return 0;
}

int a2s_players(const char *ip, uint16_t port, A2SPlayer *out, int max)
{
    int fd = make_socket(3000);
    if (fd < 0) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    uint8_t req[9] = "\xFF\xFF\xFF\xFF\x55\xFF\xFF\xFF\xFF";
    uint8_t buf[A2S_BUF_SIZE];

    int n = send_recv(fd, &addr, req, 9, buf, sizeof(buf));
    if (n < 5) { close(fd); return -1; }

    if (buf[4] == A2S_CHALLENGE) {
        memcpy(req + 5, buf + 5, 4);
        n = send_recv(fd, &addr, req, 9, buf, sizeof(buf));
    }
    close(fd);

    if (n < 6) return -1;
  
    int off   = 5;
    int count = read_byte(buf, &off);
    if (count > max) count = max;

    for (int i = 0; i < count; i++) {
        out[i].index    = read_byte(buf, &off);
        read_string(buf, &off, out[i].name, 256);
        out[i].score    = read_long(buf, &off);
        out[i].duration = read_float(buf, &off);
    }

    return count;
}
