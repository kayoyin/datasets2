#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"

char* msgtype[] = {"FYI", "MYM", "END", "TXT", "MOV"};

void show_bytes(char* buffer, int len) {
	for (int i=0; i<len; i++) {
		printf("%3d ", (int)buffer[i]);
	}
	printf("\n");
}

void send_msg(char type, int dest, int length, char* payload, struct sockaddr_in* sockstr, socklen_t* len_sock) {
  int size_msg = length + sizeof(char); //sizeof(char)=1
  char* msg = malloc(size_msg);
  char* ite = msg;

  *ite = type; // FYI MYM END etc
  ite++;
  memcpy(ite, payload, length);

  // Specific UDP
  sendto(dest, (const char *)msg, size_msg, 0, (const struct sockaddr *) sockstr, *len_sock);

  free(msg);
  printf("     [t] [%3s] (to %d) \n", msgtype[type-1], dest);
}

char* build_end(unsigned char winner, int* bslen) {
	*bslen = 1*sizeof(char);
	char* payload = malloc(*bslen);
	char *ite = payload;
	*ite = winner;

	return payload;
}

char* build_fyi(struct Position** grid, int npos, int* bslen) {
	*bslen = sizeof(char) + npos * 3 * sizeof(char);
	char* payload = malloc(*bslen);
	char* it = payload;
	*it = (char) npos;
	it++;
	for (int i = 0; i < npos; i ++) {
		memcpy(it, &(grid[i]->player), sizeof(char));
		memcpy(it+1, &(grid[i]->col), sizeof(char));
		memcpy(it+2, &(grid[i]->row), sizeof(char));

		it += 3*sizeof(char); //sizeof(Position);
	}

	return payload;
}

char* build_mov(char col, char row, int* bslen) {
	*bslen = 2*sizeof(char);
	char* payload = malloc(*bslen);
	char *ite = payload;
	*ite = col;
	ite++;
	*ite = row;

	return payload;
}

int pktlen(int type, int* params) {
	switch(type) {
		case FYI:
			// npos = params[0]
			if (!params) printf("Error, missing parameter in pktlen FYI.\n");
			return sizeof(char) + params[0] * 3 * sizeof(char);
		case MOV:
			return 2*sizeof(char);
		case END:
			return 1*sizeof(char);
		default:
			printf("Error, unknown type of packet");
			return -1;
	}
}

int fill_end(unsigned char winner, char* p) {
	int length = pktlen(END, NULL); //1*sizeof(char);
	char *ite = p;
	*ite = winner;

	return length;
}

int fill_fyi(struct Position** grid, int npos, char* p) {
	int length = pktlen(FYI, &npos); //sizeof(char) + npos * 3 * sizeof(char);
	char* it = p;
	*it = (char) npos;
	it++;
	for (int i = 0; i < npos; i ++) {
		memcpy(it, &(grid[i]->player), sizeof(char));
		memcpy(it+1, &(grid[i]->col), sizeof(char));
		memcpy(it+2, &(grid[i]->row), sizeof(char));

		it += 3*sizeof(char); //sizeof(Position);
	}

	return length;
}

int fill_mov(char col, char row, char* p) {
	int length = pktlen(MOV, NULL); //2*sizeof(char);
	char *ite = p;
	*ite = col;
	ite++;
	*ite = row;

	return length;
}
