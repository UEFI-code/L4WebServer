#define _CRT_SECURE_NO_WARNINGS
#include "toolgets.h"

const char http_header_template_200[] = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n";
const char http_header_template_404[] = "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n";

const char html_template[] = "<html><body><h1>Hello from C!</h1><br>You are %d.%d.%d.%d:%d</body></html>";

void reply_http_req(SOCKET guestSocket, struct sockaddr_in *guestAddr, char *http_rx, uint32_t rx_len, char *http_tx) {
    uint8_t* guestIP = (uint8_t*)&guestAddr->sin_addr.S_un.S_addr;
    printf("------ Dumping HTTP Request ------\n");
    fwrite(http_rx, 1, rx_len, stdout);
    printf("\n----------------------------------\n");
    // Dispatch the HTTP request
    if (strncmp(http_rx, "GET / ", 6) == 0) {
        // http_rx buffer is useless now, we can use it as scratch space
        uint32_t html_len = sprintf_s(http_rx, RX_PayloadSize, html_template, guestIP[0], guestIP[1], guestIP[2], guestIP[3], ntohs(guestAddr->sin_port));
        sprintf_s(http_tx, TX_PayloadSize, http_header_template_200, html_len);
        strcat_s(http_tx, TX_PayloadSize, http_rx);
        goto send;
    }
    // 404 Not Found
    sprintf_s(http_tx, TX_PayloadSize, http_header_template_404, 4);
    strcat_s(http_tx, TX_PayloadSize, "404");
    send:
    if (send(guestSocket, http_tx, strlen(http_tx) + 1, 0) == SOCKET_ERROR)
        printf("send failed: %d\n", WSAGetLastError());
    else
        printf("Sent payload to the guest!\n");
    return;
}