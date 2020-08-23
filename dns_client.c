#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>

int main(void) {
	printf("Command : add, get, del\n");

	while (1) {

		printf("\nPut your command:\n");
		char hostname[5];
		char command[10];
		char address[20];
		char null_address[] = "null";
		char delimeter[] = " ";
		scanf("%s %s", command, hostname);
		if (strcmp(command, "add") == 0 || strcmp(command, "del") == 0) {
			//printf("Enter IP address of host you want to add: ");
			scanf("%s", address);
		}

		if (strcmp(command, "add") == 0 || strcmp(command, "del") == 0) {
			strcat(command, delimeter);
			strcat(command, hostname);
			strcat(command, delimeter);
			strcat(command, address);
		}
		else {
			strcat(command, delimeter);
			strcat(command, hostname);
			strcat(command, delimeter);
			strcat(command, null_address);
		}
		//printf("%s", command);
		char buff_rcv[1024];

		int sock;

		struct sockaddr_in server_addr;
		int server_addr_size;

		sock = socket(PF_INET, SOCK_DGRAM, 0); 

		if (sock == -1) {
			printf("socket creation error");
		}

		memset(&server_addr, 0, sizeof(server_addr));

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(12345); 
		server_addr.sin_addr.s_addr = inet_addr("10.1.1.1");

		sendto(sock, command, 1024, 0, (struct sockaddr*) & server_addr, sizeof(server_addr));
		server_addr_size = sizeof(server_addr);

		recvfrom(sock, buff_rcv, 1024, 0, (struct sockaddr*) & server_addr, &server_addr_size);
		printf("%s\n", buff_rcv);
	}
}