#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "common.h"

int main(int argc, char** argv) {
    /* Start of code obtained from
    https://www.geeksforgeeks.org/udp-server-client-implementation-c/
    */

    int terminate = 0;  // run until told otherwise
    // Arguments parsing
    if (argc<3) {
        printf("Syntax: ./git client <ipaddr> <port>\n");
        exit(1);
    }
    int port = atoi(argv[2]);
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    memset(&cliaddr, 0, sizeof(struct sockaddr_in));

    inet_pton(AF_INET, argv[1], &(servaddr.sin_addr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    // Socket creation
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // creation of datagram socket
    if (sockfd < 0) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
