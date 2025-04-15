#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdint.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

typedef union {
    uint32_t addr;
    uint8_t addr_bytes[4];
} ipv4_addr;

typedef struct
{
    UINT8 status; // 0x44 is broken pipe, 0x55 is normal
} ctlHeader;

#define PayloadSize 1024

extern SOCKET init_winsock(UINT16 bindPort);

extern void dump_packet(UINT8* packet, UINT32 packetSize);