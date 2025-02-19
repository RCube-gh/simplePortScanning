#include <netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<arpa/inet.h>
#include<unistd.h>

int main(){
	int sock=socket(AF_INET,SOCK_RAW, IPPROTO_TCP);
	if(sock<0){
		perror("[-] Error creating raw socket\n");
		exit(1);
	}
	printf("[+] Raw socket created successfully\n");
	close(sock);
	return 0;
}


