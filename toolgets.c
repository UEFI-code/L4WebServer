#include "toolgets.h"

WSADATA wsaData = { 0 };
BOOL WSAStartupDone = FALSE;

struct sockaddr_in TxAddr = { 0 };

SOCKET init_winsock(UINT16 bindPort) {
    SOCKET L4Socket = NULL;
    if (!WSAStartupDone) {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { // 2.2 should work on Windows 95 and later
            printf("WSAStartup failed.\n");
            exit(1);
        }
        WSAStartupDone = TRUE;
    }

    L4Socket = socket(AF_INET, SOCK_STREAM, 0);
    if (L4Socket == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    struct sockaddr_in RxAddr = { 0 };

    RxAddr.sin_family = AF_INET;
    RxAddr.sin_port = bindPort;
    RxAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(L4Socket, (struct sockaddr*)&RxAddr, sizeof(RxAddr)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(L4Socket);
        WSACleanup();
        L4Socket = NULL;
        exit(1);
    }
    
    return L4Socket;
}

void set_sock_timeout(SOCKET sock, uint32_t timeout) {
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(uint32_t)) == SOCKET_ERROR) {
        printf("setsockopt failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        exit(1);
    }
}

void dump_packet(UINT8* packet, UINT32 packetSize) {
    printf("------ Packet Dump ------\n");
    for (UINT32 i = 0; i < packetSize; i++) {
        printf("%02X ", packet[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n------------\n");
}