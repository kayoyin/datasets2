#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "common.h"

void printboard(Game* game);


int main(int argc, char** argv) {
    int terminate = 0;  // run until told otherwise
    // Arguments parsing
    if (argc<3) {
        printf("Syntax: ./client <ipaddr> <port>\n");
        exit(1);
    }
    int port = atoi(argv[2]);
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(struct sockaddr_in));
    inet_pton(AF_INET, argv[1], &(dest.sin_addr));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    // Socket creation
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // creation of datagram socket
    if (sockfd < 0) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    // Greetings message
    char *msg = "Let's play! \n";
    if (sendto(sockfd, (const char *)msg, strlen(msg),
        0, (const struct sockaddr *) &dest,
            sizeof(dest)) < 0){
              perror("greeting failed");
    }

    // Loop
    char buffer[1024];
    char msgtype;
    Game* game = NULL;
    int col, row;
    while (! terminate) {
        // Receive message from server
        socklen_t len = sizeof(dest);
        int n = recvfrom(sockfd, (char *)buffer, 1024,
                0, (struct sockaddr *) &dest,
                &len);

        // Process message (and maybe interact with user? send message(s) to server?)
        if (n == 0) break;

        buffer[n] = '\0';
        msgtype = buffer[0];
        show_bytes(buffer, sizeof(buffer));
        if (msgtype == TXT){
          printf("TXT message : %s\n", buffer);
        }
        else if (msgtype == FYI){
          printboard(game);
        }
        else if (msgtype == MYM){

          while (1){
            printf("Please choose your next move (choose a position on the board) \n");
            scanf("%d, %d", &col, &row);
            if (col < 0 || col > 8 || row < 0 || row > 0){
              printf("Please choose a valid board position \n");
              continue;
            }
            /*
            else if (!isspace(board[col][row])){
              printf("Please choose a free board position \n");
              continue;
            }
            */
            int* bslen = NULL;
            msg = build_mov(col, row, bslen);
            break;
          }
          /*
          sendto(sockfd, (const char *)msg, strlen(msg),
              0, (const struct sockaddr *) &dest,
                  sizeof(dest))
          */
          send_msg(MOV, sockfd, strlen(msg), msg, &dest, sizeof(dest));

        }
        else if (msgtype == END)
        {
          printf("end TBD");
          terminate = 1;
        }

    }
    // Terminate
    close(sockfd);
    return 0;
}

void printboard(Game* game){
  Position** grid = game->grid;
  printf("%d filled positions. \n", game->npos);
  printf("%c | %c | %c \n", grid[0]->player, grid[1]->player, grid[2]->player);
  printf("-+-+- \n");
  printf("%c | %c | %c \n", grid[3]->player, grid[4]->player, grid[5]->player);
  printf("-+-+- \n");
  printf("%c | %c | %c \n", grid[6]->player, grid[7]->player, grid[8]->player);
}
