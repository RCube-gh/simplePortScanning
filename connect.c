#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void scan_port(const char *ip, int port) {
    int sock;
    struct sockaddr_in target;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr *)&target, sizeof(target)) == 0) {
        printf("[+] Port %d is open\n", port);
    } else {
        // printf("[-] Port %d is closed\n", port);  // Uncomment if you want to see closed ports too
    }

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <start-port>-<end-port>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int start_port, end_port;

    if (sscanf(argv[2], "%d-%d", &start_port, &end_port) != 2) {
        printf("Invalid port range format. Use: start-end (e.g., 20-100)\n");
        return 1;
    }

    for (int port = start_port; port <= end_port; port++) {
        scan_port(ip, port);
    }

    return 0;
}

//powered by chatgpt
