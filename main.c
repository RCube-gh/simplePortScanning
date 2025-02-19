#include <netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<arpa/inet.h>
#include<unistd.h> //close()


struct ipheader{
	unsigned char ihl:4, version:4; //version 4, header length
	unsigned char tos; //type of service (default:0)
	unsigned short tot_len; //total packet length
	unsigned short id; //packet id
	unsigned short frag_off; //fragment offset
	unsigned char ttl; //ttl
	unsigned char protocol; //protocol (tcp=6)
	unsigned short check;// checksum
	unsigned int saddr; //source ip
	unsigned int daddr; //destination id
};

//calculate checksum
unsigned short checksum(void *b,int len){
	unsigned short *buf=b;
	unsigned int sum=0;
	unsigned short result;

	for(sum=0;len>1;len-=2){
		sum+=*buf++;
	}
	if(len==1){
		sum+=*(unsigned char*)buf;
	}
	sum=(sum>>16)+(sum&0xFFFF);
	sum+=(sum>>16);
	result=~sum;
	return result;
}


int main(int argc, char *argv[]){
	if(argc!=3){
		printf("Usage: %s <source IP> <destination IP>\n",argv[0]);
		return 1;
	}
	char *src_ip=argv[1];
	char *dst_ip=argv[2];

	int sock=socket(AF_INET,SOCK_RAW, IPPROTO_TCP);
	if(sock<0){
		perror("[-] Error creating raw socket\n");
		exit(1);
	}
	printf("[+] Raw socket created successfully\n");

	//allocate memory for packet
	char packet[sizeof(struct ipheader)];
	memset(packet,0,sizeof(packet));

	struct ipheader *ip=(struct ipheader *)packet;
	ip->ihl=5;//default ip header length 5*4=20bytes
	ip->version=4;
	ip->tos=0;
	ip->tot_len=htons(sizeof(struct ipheader));
	ip->id=htons(54321);//random packet id
	ip->frag_off=0;//no fragmentation
	ip->ttl=64;
	ip->protocol=IPPROTO_TCP;
	ip->saddr=inet_addr(src_ip);
	ip->daddr=inet_addr(dst_ip);
	ip->check=checksum((unsigned short *)packet, sizeof(struct ipheader));
	printf("[+] IP header created successfully\n");
	printf("[+] IP: %s -> %s\n",src_ip,dst_ip);

	close(sock);
	return 0;
}


