#include <netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<arpa/inet.h>
#include<unistd.h> //close()
#include<time.h> //random


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

struct tcpheader{
    unsigned short source;
    unsigned short dest;
    unsigned int seq;
    unsigned int ack_seq;
    unsigned char res1:4,doff:4;
    unsigned char flags;
    unsigned short window;
    unsigned short check;
    unsigned short urg_ptr;
};

//used for tcp checksum calculation
struct pseudo_header{
    unsigned int saddr;
    unsigned int daddr;
    unsigned char reserved;
    unsigned char protocol;
    unsigned short tcp_len;
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
	if(argc!=4){
		printf("Usage: %s <source IP> <destination IP> <destination port>\n",argv[0]);
		return 1;
	}
	char *src_ip=argv[1];
	char *dst_ip=argv[2];
    int dst_port=atoi(argv[3]);

	int sock=socket(AF_INET,SOCK_RAW, IPPROTO_TCP);
	if(sock<0){
		perror("[-] Error creating raw socket\n");
		exit(1);
	}
	printf("[+] Raw socket created successfully\n");
    int one=1;
    if(setsockopt(sock,IPPROTO_IP,IP_HDRINCL,&one,sizeof(one))<0){
        perror("[-] Error setting socket options");
        exit(1);
    }

	//allocate memory for packet (IP+TCP header)
	char packet[sizeof(struct ipheader)+sizeof(struct tcpheader)];
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

    struct tcpheader *tcp=(struct tcpheader *)(packet+sizeof(struct ipheader));
    srand(time(0));//random source port
    //tcp->source=htons(rand()%65535);
    tcp->source=htons(12345);
    tcp->dest=htons(dst_port);
    tcp->seq=htonl(0);//initial sequence number
    tcp->ack_seq=0;
    tcp->doff=5;//tcp header size
    tcp->flags=0x02;//syn flag
    tcp->window=htons(1024);//window size
    tcp->check=0;
    tcp->urg_ptr=0;

    //pseudo header for tcp checksum calculation
    struct pseudo_header psh;
    psh.saddr=inet_addr(src_ip);
    psh.daddr=inet_addr(dst_ip);
    psh.reserved=0;
    psh.protocol=IPPROTO_TCP;
    psh.tcp_len=htons(sizeof(struct tcpheader));

    char pseudo_packet[sizeof(struct pseudo_header)+sizeof(struct tcpheader)];
    memcpy(pseudo_packet, &psh,sizeof(struct pseudo_header));
    memcpy(pseudo_packet+sizeof(struct pseudo_header),tcp,sizeof(struct tcpheader));
    tcp->check=checksum((unsigned short*)pseudo_packet,sizeof(pseudo_packet));
    printf("[+] TCP header created successfully\n");
    printf("[+] destination port: %d\n",dst_port);
    printf("SYN packet ready\n");

    //destination details
    struct sockaddr_in dest;
    dest.sin_family=AF_INET;
    dest.sin_port=htons(dst_port);
    dest.sin_addr.s_addr=inet_addr(dst_ip);


    //send the synn packet
    if(sendto(sock,packet,sizeof(packet),0,(struct sockaddr*)&dest, sizeof(dest))<0){
        perror("[-] Error sending packet");
        exit(1);
    }
    printf("[+] SYN packet sent to %s:%d\n",dst_ip,dst_port);




    struct timeval timeout;
    timeout.tv_sec=3;
    timeout.tv_usec=0;
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    char buffer[1024];
    struct sockaddr_in sender_addr;
    socklen_t addr_len=sizeof(sender_addr);





    printf("[+] Listening for responses...\n");
    ssize_t recv_len=recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr *)&sender_addr,&addr_len);
    if(recv_len<0){
        printf("[-] No response (port is filtered or host is down)\n");
    }else{
        struct ipheader *ip_resp=(struct ipheader *)buffer;
        int ip_header_len=ip_resp->ihl*4;
        struct tcpheader *tcp_resp=(struct tcpheader *)(buffer+ip_header_len);

        if(ntohs(tcp->dest)==dst_port){
            if(tcp->flags==0x12){
                printf("[+] Port %d is OPEN!\n",dst_port);
            }else if(tcp->flags==0x14){
                printf("[-] Port %d is CLOSED!\n",dst_port);
            }
        }
    }

	close(sock);
	return 0;
}


