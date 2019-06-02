#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "common.h"

int validate_move(int col, int row, Position** grid, int npos){
  if (col < 1 || col > 3 || row < 1 || row > 3){
    return 0; // position not in 3x3 grid
  }
  for (int i = 0; i < npos; i++){
    if (grid[i]->row == row && grid[i]->col == col){
      return 0;
    }
  }
  return 1;
}

int check_winner(Position** grid, int npos){
  // return -1 if not end, 1 if player 1 wins
  // 2 if player 2 wins, 0 if tie
  char lrow = grid[npos]->row;
  char lcol = grid[npos]->col;
  char lplay = grid[npos]->player;
  char row;
  char col;

  int rows = 0;
  int cols = 0;
  int diag1 = 0;
  int diag2 = 0;

  for (int i = (int)lplay - 1; i < npos+1; i = i+2){
    row = grid[i]->row;
    col = grid[i]->col;

    if (row == lrow){
      rows++;
    }
    if (col == lcol){
      cols++;
    }
    if (lrow == lcol){
      if (row == col){
        diag1++;
      }
    }
    if ((int)lrow + (int)lcol == 4){
      if ((int)row + (int)col == 4){
        diag2++;
      }
    }
  }

  if (rows == 3 || cols == 3 || diag1 == 3 || diag2 == 3){
    return (int)lplay;
  }

  if (npos == 8){
    return 0;
  }

  return -1;
}

int main(int argc, char** argv) {
    /* Start of code based on
    https://www.geeksforgeeks.org/udp-server-client-implementation-c/
    */

    struct Game game;

    // Arguments parsing
    if (argc<2) {
        printf("Syntax: ./server <port>\n");
        exit(1);
    }
    int port = atoi(argv[1]);
    struct sockaddr_in servaddr, cliaddr;

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    memset(&cliaddr, 0, sizeof(cliaddr));

    inet_pton(AF_INET, argv[1], &(servaddr.sin_addr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    // Socket creation
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // creation of datagram socket
    //int sockpl[2] = {0,0}; // socket for each player
    if (sockfd < 0) {
      perror("socket creation failed");
      exit(EXIT_FAILURE);
    }

    printf("Socket created.\n");


    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Bind to port %d \n", port);
    printf("Waiting for connections...\n");

    char buffer[2048];
    char symbols[2] = {'X', 'O'};
    socklen_t len = sizeof(cliaddr);
    char *msg;

    game.nplayers = 0;
    while (game.nplayers < 2){
      /*
      if (listen(sockfd, 2) < 0)
      {
          perror("listen");
          exit(EXIT_FAILURE);
      }
      memset(&cliaddr, 0, sizeof(struct sockaddr_in));
      socklen_t clilen = sizeof(cliaddr);
      if ((sockpl[nplayers] = accept(sockfd,(struct sockaddr *) &cliaddr,&clilen) < 0))
      {
        perror("accept");
        exit(EXIT_FAILURE);
      }*/
      int n = recvfrom(sockfd, (char *)buffer, 2048,
                0, ( struct sockaddr *) &cliaddr,
                &len);
      buffer[n] = '\0';
      printf("%s\n", buffer);
      game.nplayers++;
      printf("Player %d has connected \n", game.nplayers);

      game.player[game.nplayers-1].socket = cliaddr;
      game.player[game.nplayers-1].len_sock = len;
      printf("Player %d assigned. \n", game.nplayers);

      asprintf(&msg, " Welcome! You are player %d, you play with %c. \n", game.nplayers, symbols[game.nplayers-1]);
      send_msg(TXT, sockfd, strlen(msg), msg, &cliaddr, &len);
    }


    game.active = 1;
    game.turn = 0;
    int bslen = 0;
    game.npos = 0;
    Position pos[9];
    struct sockaddr_in psocket;
    socklen_t psocklen;
    char col,row;

    while (game.active) {
      psocket = game.player[game.turn].socket;
      psocklen = game.player[game.turn].len_sock;
      msg = build_fyi(game.grid, game.npos, &bslen);
      send_msg(FYI, sockfd, strlen(msg), msg, &psocket, &psocklen);

      int validmov = 0;
      while (!validmov){
        send_msg(MYM, sockfd, strlen(msg), msg, &psocket, &psocklen);
        int n = recvfrom(sockfd, (char *)buffer, 2048,
                  0, ( struct sockaddr *) &psocket, &psocklen);
        buffer[n] = '\0';
        col = (int)buffer[1];
        row = (int)buffer[2];
        validmov = validate_move(col,row, game.grid, game.npos);
        if (!validmov){
          msg = "Invalid move! Try again. \n";
          send_msg(TXT, sockfd, strlen(msg), msg, &psocket, &psocklen);
        }
      }
      printf("Received move is col=%d, row=%d. \n", col, row);
      pos[game.npos].player = game.turn + 1;
      pos[game.npos].col = col;
      pos[game.npos].row = row;
      game.grid[game.npos] = &pos[game.npos];

      /*
      game.grid[game.npos]->player = game.turn + 1;
      game.grid[game.npos]->col = col;
      game.grid[game.npos]->row = row;
      */
      printf("Position inserted \n");

      int res = check_winner(game.grid, game.npos);

      if (res > -1){
        msg = build_end(res, &bslen);
        send_msg(END, sockfd, strlen(msg), msg, &psocket, &psocklen);
        game.active = 0;
      }

      game.npos++;
      game.turn = (game.turn + 1) % 2;
    }
    psocket = game.player[game.turn].socket;
    psocklen = game.player[game.turn].len_sock;
    send_msg(END, sockfd, strlen(msg), msg, &psocket, &psocklen);

    close(sockfd);
    return 0;
}
