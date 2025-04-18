#include "toolgets.h"
#include "http_processor.h"

UINT16 myExposePort = 8080;

SOCKET externalSocket = NULL;

HANDLE workerThread = NULL;

__inline SOCKET waitGuestOnline()
{
    printf("Waiting for the guest online...\n");
    SOCKET guestSocket = accept(externalSocket, NULL, NULL);
    if (guestSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(externalSocket);
        WSACleanup();
        exit(1);
    }
    printf("The Guest is online!\n");
	return guestSocket;
}

void extern_worker()
{
    UINT8 *rx_payload = (UINT8*)malloc(RX_PayloadSize); UINT32 recvSize = 0;
    UINT8 *tx_payload = (UINT8*)malloc(TX_PayloadSize); UINT32 sendSize = 0;
    struct sockaddr_in guestAddr = { 0 }; int guestAddrSize = sizeof(guestAddr);
    while (1)
    {
        SOCKET guestSocket = waitGuestOnline();
        getpeername(guestSocket, (struct sockaddr*)&guestAddr, &guestAddrSize);
        uint8_t* guestIP = (uint8_t*)&guestAddr.sin_addr.S_un.S_addr;
		printf("The guest IP is %d.%d.%d.%d:%d\n", guestIP[0], guestIP[1], guestIP[2], guestIP[3], ntohs(guestAddr.sin_port));
        set_sock_timeout(guestSocket, 5000);
        recvSize = recv(guestSocket, rx_payload, RX_PayloadSize, 0);
        if (recvSize == SOCKET_ERROR) {
            printf("recv failed: %d\n", WSAGetLastError());
            goto fin;
        }
        printf("Recv from the guest:\n"); dump_packet(rx_payload, recvSize);
        reply_http_req(&guestAddr, (char*)rx_payload, recvSize, (char*)tx_payload, &sendSize);
        if (send(guestSocket, tx_payload, sendSize, 0) == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            goto fin;
        }
        printf("Sent payload to the guest!\n");
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