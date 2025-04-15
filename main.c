#include "toolgets.h"
#include "http_template.h"

UINT16 myExposePort = 8080;
ipv4_addr TheGuestSrcIP = { 0 }; UINT16 TheGuestSrcPort = 0;

SOCKET externalSocket = NULL, guestSocket = NULL;

HANDLE workerThread = NULL;

__inline void waitGuestOnline()
{
    printf("Waiting for the guest online...\n");
    guestSocket = accept(externalSocket, NULL, NULL);
    if (guestSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(externalSocket);
        WSACleanup();
        exit(1);
    }
    printf("The Guest is online!\n");
    // now, get the guest IP
    struct sockaddr_in guestAddr = { 0 };
    int guestAddrSize = sizeof(guestAddr);
    getpeername(guestSocket, (struct sockaddr*)&guestAddr, &guestAddrSize);
    TheGuestSrcIP.addr = guestAddr.sin_addr.S_un.S_addr;
    // get the guest port
    TheGuestSrcPort = guestAddr.sin_port;
    printf("The Guest is %d.%d.%d.%d:%d\n", TheGuestSrcIP.addr_bytes[0], TheGuestSrcIP.addr_bytes[1], TheGuestSrcIP.addr_bytes[2], TheGuestSrcIP.addr_bytes[3], ntohs(TheGuestSrcPort));
}

void extern_worker()
{
    UINT8 *payload = (UINT8*)malloc(PayloadSize);
    UINT32 recvSize = 0;
    while (1)
    {
        if(guestSocket == NULL)
        {
            waitGuestOnline();
        }
        recvSize = recv(guestSocket, payload, PayloadSize, 0);
        if (recvSize == SOCKET_ERROR) {
            printf("recv failed: %d\n", WSAGetLastError());
            goto fin;
        }
        printf("Recv from the guest:\n");
        dump_packet(payload, recvSize);
        // simply reply with http_template
        if (send(guestSocket, http_template, strlen(http_template) + 1, 0) == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            goto fin;
        }
        printf("Sent template to the guest\n");
        fin:
        closesocket(guestSocket); guestSocket = NULL;
    }
}

int main()
{
    printf("Will listen on port %d\n", myExposePort);
    // convert all port number using htons
    myExposePort = htons(myExposePort);
    // now we should create the socket
    externalSocket = init_winsock(myExposePort);
    // now, listen on the externalSocket
    if (listen(externalSocket, 1) == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(externalSocket);
        WSACleanup();
        exit(1);
    }
    printf("Listening on the externalSocket...\n");
    // OK, start the loop
    while(1)
    {
        if (workerThread == NULL) {
            workerThread = CreateThread(NULL, 0, extern_worker, NULL, 0, NULL);
        }
        Sleep(1000);
    }
}