#define _CRT_SECURE_NO_WARNINGS
#include "toolgets.h"

char http_header_template_200[] = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n";
char http_header_template_404[] = "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n"
                        "\r\n";

char html_template[] = "<html><body><h1>Hello from C!</h1><br>You are %d.%d.%d.%d:%d</body></html>";

void reply_http_req(struct sockaddr_in *guestAddr, char *http_req, uint32_t req_len, char *http_res, uint32_t *res_len) {
    uint8_t* guestIP = (uint8_t*)&guestAddr->sin_addr.S_un.S_addr;
    printf("------ Dumping HTTP Request ------\n");
    fwrite(http_req, 1, req_len, stdout);
    printf("\n----------------------------------\n");

    if (strncmp(http_req, "GET / ", 6) != 0) {
        // 404 Not Found
        sprintf_s(http_res, TX_PayloadSize, http_header_template_404, 4);
        strcat_s(http_res, TX_PayloadSize, "404");
        *res_len = strlen(http_res) + 1;
        return;
    }
    
    char buf[512] = { 0 };
    uint32_t content_len = sprintf_s(buf, sizeof(buf), html_template, guestIP[0], guestIP[1], guestIP[2], guestIP[3], ntohs(guestAddr->sin_port));

    sprintf_s(http_res, TX_PayloadSize, http_header_template_200, content_len);
    strcat_s(http_res, TX_PayloadSize, buf);
    
    *res_len = strlen(http_res) + 1;
}