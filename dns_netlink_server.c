#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NETLINK_USER 31

// Type value used in kernel message
#define MSG_REGISTER 1
#define MSG_REGISTER_RESPONSE 2
#define MSG_DEREGISTER 3
#define MSG_DEREGISTER_RESPONSE 4
#define MSG_GET 5
#define MSG_GET_RESPONSE 6

// Code value used in kernel message
#define MSG_SUCCESS 0
#define MSG_FAILED 1

#define SET_MSG_CHAR(name, x) (*name = x); (name += 1);
#define SET_MSG_SHORT(name, x) (*(unsigned short *)name = htons(x)); (name += 2);
#define SET_MSG_INTEGER(name, x) (*(unsigned int *)name = htonl(x)); (name += 4);
#define SET_MSG_STRING(name, x) (strcpy(name, x)); (name += strlen(x)+1);
#define MAX_PAYLOAD 1024

#define reg_success "New Host Registration Success"
#define reg_fail "Registration Failed - Hostname already exists"
#define dereg_success "De-Registration Success"
#define dereg_fail "De-Registration Failed. No entry"
#define get_data_fail "Get data Failed. No entry"
#define MyIdentification "Hi, hello world : kim jaehun"


int sock_fd; // netlink socket

unsigned short set_add_message(char* name, unsigned int ipv4_addr, char* hdr_pointer) {
	unsigned short msg_len = 7 + strlen(name) + 1; // default kernel request header length 3 + sizeof(integer) + strlen(name) + NULL Value for string => 7 + strlen(name) + 1
	SET_MSG_CHAR(hdr_pointer, MSG_REGISTER)
		SET_MSG_SHORT(hdr_pointer, msg_len)
		SET_MSG_INTEGER(hdr_pointer, ipv4_addr)
		SET_MSG_STRING(hdr_pointer, name)

		return msg_len;
}

unsigned short set_del_message(char* name, unsigned int ipv4_addr, char* hdr_pointer) {
	unsigned short msg_len = 7 + strlen(name) + 1;
	// default kernel request header length 3 + sizeof(integer) + strlen(name) + NULL Value for string => 7 + strlen(name) + 1
	SET_MSG_CHAR(hdr_pointer, MSG_DEREGISTER)
		SET_MSG_SHORT(hdr_pointer, msg_len)
		SET_MSG_INTEGER(hdr_pointer, ipv4_addr)
		SET_MSG_STRING(hdr_pointer, name)

		return msg_len;
}

unsigned short set_get_message(char* name, char* hdr_pointer) {
	unsigned short msg_len = 3 + strlen(name) + 1;
	// default kernel request header length 3 + strlen(name) + NULL Value for string => 3 + strlen(name) + 1
	SET_MSG_CHAR(hdr_pointer, MSG_GET)
		SET_MSG_SHORT(hdr_pointer, msg_len)
		SET_MSG_STRING(hdr_pointer, name)

		return msg_len;
}


unsigned short send_add_message(char* name, unsigned int ipv4_addr) {
	
	char* d_point;
	char* additional_data_ptr;

	unsigned short rcv_len;

	struct nlmsghdr* nlh = NULL;

	char buf[NLMSG_SPACE(MAX_PAYLOAD)];

	unsigned int host_ipv4 = htonl(ipv4_addr);
	memset(buf, 0, NLMSG_SPACE(MAX_PAYLOAD));

	nlh = (struct nlmsghdr*)buf;
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	printf("Sending Registration message(%s, %s) to kernel\n", name, inet_ntoa(*(struct in_addr*) & host_ipv4));
	set_add_message(name, ipv4_addr, (char*)NLMSG_DATA(nlh));

	send(sock_fd, buf, nlh->nlmsg_len, 0);
	printf("Waiting for message from kernel\n");

	
	recv(sock_fd, buf, nlh->nlmsg_len, 0);
	// response from kernel is saved to buf
	
	d_point = (char*)NLMSG_DATA(nlh);// get data starting pointer to process received data

	
	//MSG_REGISTER_RESPONSE => type = 2
	//MSG_SUCCESS or MSG_FAILED => code = 0 or 1
	//length => 04 
	int kernel_response_buf[8];
	for (int i = 0; i < 4; i++) {
		//printf("%d\n", *(d_point+i));
		kernel_response_buf[i] = *(d_point + i);
	}
	for (int j = 0; j < 4; j++) {
	//	printf("%d", kernel_response_buf[j]);
	}
	int response_code = kernel_response_buf[1];
	if (response_code == 0) {
		printf("Registration Success\n");
		return 0;
	}
	else {
		printf("Registration Failed - Hostname already exists\n");
		return 1;
	}
}

unsigned short send_del_message(char* name, unsigned int ipv4_addr, char* buf_rcv) {
	// buf_rcv : response message buffer for client

	char* d_point;
	char* additional_data_ptr;
	unsigned short rcv_len;

	struct nlmsghdr* nlh = NULL;

	char buf[NLMSG_SPACE(MAX_PAYLOAD)];

	unsigned int host_ipv4 = htonl(ipv4_addr);
	memset(buf, 0, NLMSG_SPACE(MAX_PAYLOAD));

	nlh = (struct nlmsghdr*)buf;
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	printf("Sending De-registration message(%s, %s) to kernel\n", name, inet_ntoa(*(struct in_addr*) & host_ipv4));
	set_del_message(name, ipv4_addr, (char*)NLMSG_DATA(nlh));

	send(sock_fd, buf, nlh->nlmsg_len, 0);
	printf("Waiting for message from kernel\n");

	
	recv(sock_fd, buf, nlh->nlmsg_len, 0);
	d_point = (char*)NLMSG_DATA(nlh); // get data starting pointer to process received data

	
	int kernel_response_buf[8];
	for (int i = 0; i < 4; i++) {
		//printf("%d\n", *(d_point+i));
		kernel_response_buf[i] = *(d_point + i);
	}
	for (int j = 0; j < 4; j++) {
			//printf("%d", kernel_response_buf[j]);
	}
	int response_code = kernel_response_buf[1];
	if (response_code == 0) {
		printf("De-Registration Success\n");
		return 0;
	}
	else {
		printf("De-Registration Failed. No entry\n");
		return 1;
	}
}

unsigned short send_get_message(char* name, char* lookup_addr) {
	// buf_rcv : response message buffer for client.

	char* d_point;
	char* additional_data_ptr;

	unsigned short rcv_len;

	struct nlmsghdr* nlh = NULL;

	char buf[NLMSG_SPACE(MAX_PAYLOAD)];

	memset(buf, 0, NLMSG_SPACE(MAX_PAYLOAD));

	nlh = (struct nlmsghdr*)buf; 

	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD); // set message length
	nlh->nlmsg_pid = getpid(); // set my pid
	nlh->nlmsg_flags = 0; // set flags

	printf("Sending Translation Request(%s -> IPv4) message to kernel\n", name);

	set_get_message(name, (char*)NLMSG_DATA(nlh));
	send(sock_fd, buf, nlh->nlmsg_len, 0); // send data

	printf("Waiting for message from kernel\n");

	

	recv(sock_fd, buf, nlh->nlmsg_len, 0); // receive
	d_point = (char*)NLMSG_DATA(nlh); // get data starting pointer to process received data
	
	//MSG_GET_RESPONSE => type = 6
	//MSG_SUCCESS or MSG_FAILED => code = 0 or 1
	//length = 08
	//IP address follows length
	int response_buf[8];
	for (int i = 0; i < 8; i++) {
		//printf("%d\n", *(d_point+i));
		response_buf[i] = *(d_point + i);
	}
	
	int response_code = response_buf[1];
	
	if (response_code == 0) {
		for (int j = 4; j < 8; j++) {
			char num[2];
			sprintf(num, "%d", response_buf[j]);
			strcat(lookup_addr, num);
			if (j != 7) { 
				strcat(lookup_addr, "."); 
			}
		}
		printf("Getting data Success\n");
	}
	else {
		printf("Get data Failed. No entry\n");
	}
}



int main(void) {
	int service_sock_fd;
	struct sockaddr_nl src_addr;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int addr_len;

	service_sock_fd = socket(PF_INET, SOCK_DGRAM, 0); // Client <-> Client

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER); // VM2 <-> Module

	if (sock_fd < 0) {
		printf("vm2 <-> module : -1");
	}
	if (service_sock_fd < 0) {
		printf("client <-> client : -1");
	}


	memset(&server_addr, 0, sizeof(server_addr));
	memset(&src_addr, 0, sizeof(src_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(12345); 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // receive data regardless dest IPv4 address
	

	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

	if (bind(service_sock_fd, (struct sockaddr*) & server_addr, sizeof(server_addr)) == -1) {
		printf("bind error");
	}
	if (bind(sock_fd, (struct sockaddr*) & src_addr, sizeof(src_addr)) == -1) {
		printf("module bind error");
	}
	

	while (1) {
		addr_len = sizeof(client_addr);
		
		char buff_rcv[256];
		char request_addr[] = "";
		printf("\nWaiting for new request..\n");
		
		
		recvfrom(service_sock_fd, buff_rcv, 256, 0, (struct sockaddr*) & client_addr, &addr_len);
		printf("receive: %s\n", buff_rcv);

		
		//message parsing (command, hostname, address)

		char* ptr = strtok(buff_rcv, " ");
		char* command = ptr;
		//printf("%s\n", command);
		char* hostname = strtok(NULL, " ");
		//printf("%s\n", hostname);
		char* address = strtok(NULL, " ");
	    //printf("%s\n", address);

		unsigned int ipv4_addr = inet_addr(address);

		// Receive request from client, and process(get result from kernel and send back to client) received request

		if (strcmp(command, "add") == 0) {
			if (send_add_message(hostname, ipv4_addr) == 0) {
				sendto(service_sock_fd, reg_success, strlen(reg_success)+5, 0, (struct sockaddr*) & client_addr, addr_len);
			}
			else {
				sendto(service_sock_fd, reg_fail, strlen(reg_fail)+5, 0, (struct sockaddr*) & client_addr, addr_len);
			}
		}

		if(strcmp(command, "get") == 0) {
			send_get_message(hostname, request_addr);
			if (strcmp(request_addr, "") != 0) {
				//printf("%s address : %s\n", hostname, request_addr);
				sendto(service_sock_fd, request_addr, strlen(request_addr) + 5, 0, (struct sockaddr*) & client_addr, addr_len);
			}
			else {
				sendto(service_sock_fd, get_data_fail, strlen(get_data_fail) + 5, 0, (struct sockaddr*) & client_addr, addr_len);
			}
		}

		if (strcmp(command, "del") == 0) {
			if (send_del_message(hostname, ipv4_addr, buff_rcv) == 0) {
				sendto(service_sock_fd, dereg_success, strlen(dereg_success) + 5, 0, (struct sockaddr*) & client_addr, addr_len);
			}
			else {
				sendto(service_sock_fd, dereg_fail, strlen(dereg_fail) + 5, 0, (struct sockaddr*) & client_addr, addr_len);
			}
		}
		printf("----------------------------------------");
		


		
		

	}
	close(sock_fd);
}