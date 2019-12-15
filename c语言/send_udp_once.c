//
// Created by wu on 2019-12-09.
//
#include <stdio.h>
#include <netinet/ip.h>//Provides declarations for ip header
#include <netinet/udp.h>//Provides declarations for udp header
#include <arpa/inet.h>
#include <stdlib.h> //for exit(0);
#include <string.h> //memset,memcpy

//needed for checksum calculation
struct pseudo_header {
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short udp_or_tcp_length;
};

//checksum is 16bit,should use unsigned short
static unsigned short calculate_checkcsum(unsigned short *ptr, int pktlen) {
    register uint32_t csum = 0;

    //add 2 bytes / 16 bits at a time!!
    while (pktlen > 1) {
        csum += *ptr++;
        pktlen -= 2;
    }

    //add the last byte if present
    if (pktlen == 1) {
        csum += *(uint8_t *) ptr;
    }

    //add the carries
    csum = (csum >> 16) + (csum & 0xffff);
    csum = csum + (csum >> 16);

    //return the one's compliment of calculated sum
    return ((short) ~csum);
}

unsigned short src_port = 0;
unsigned int src_inet_addr = 0;
unsigned short dst_port = 0;
unsigned int dst_inet_addr = 0;
char *payload_data = NULL;

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("usage:./<script name> <src ip> <src port> <dst ip> <dst port> <data>\n");
        exit(1);
    }

    int i;
    for (i = 1; i < argc; i++) {
        if (i == 1) {
            printf("src_ip = %s\n", argv[i]);
            src_inet_addr = inet_addr(argv[i]);
            continue;
        }
        if (i == 2) {
            src_port = (unsigned short) atoi(argv[i]);
            printf("src_port = %d\n", src_port);
            continue;
        }
        if (i == 3) {
            printf("dst_ip = %s\n", argv[i]);
            dst_inet_addr = inet_addr(argv[i]);
            continue;
        }
        if (i == 4) {
            dst_port = (unsigned short) atoi(argv[i]);
            printf("dst_port = %d\n", dst_port);
            continue;
        }
        if (i == 5) {
            payload_data = argv[i];
            printf("data = %s\n", payload_data);
            continue;
        }
    }

    int iphdr_size = sizeof(struct iphdr);
    int udphdr_size = sizeof(struct udphdr);
    int payload_size = payload_data == NULL ? 0 : strlen(payload_data);
    int datagram_size = iphdr_size + udphdr_size + payload_size;
    int pseudohdr_size = sizeof(struct pseudo_header);

    // udp-datagram packet.size
    char datagram[datagram_size];
    bzero(datagram, sizeof(datagram));

    // 将 ip 首部指针和 udp 首部指针指向各自的位置，
    struct iphdr *ip_header = (struct iphdr *) datagram;
    struct udphdr *udp_header = (struct udphdr *) (datagram + iphdr_size);
    //data赋值在udp header后面
    if (payload_data != NULL) {
        strcpy(datagram + iphdr_size + udphdr_size, payload_data);
    }

    // 填充 ip 首部字段
    ip_header->ihl = 5;                     //普通 IP 数据报
    ip_header->version = 4;                 //IPv4
    ip_header->tos = 0;
    ip_header->tot_len = htons(datagram_size);
    ip_header->id = 0;                      //自定 IP 包标识，方便筛选
    ip_header->frag_off = 0;
    ip_header->ttl = 255;
    ip_header->protocol = IPPROTO_UDP;                      //指定承载的是 UDP 数据包
    ip_header->check = 0;                                //之后需要计算校验和
    ip_header->saddr = src_inet_addr;
    ip_header->daddr = dst_inet_addr;

    //计算 ip 校验和，结果为两个字节
    ip_header->check = calculate_checkcsum((unsigned short *) datagram, datagram_size);

    // 填充 udp 首部字段
    udp_header->source = htons(src_port);   //htons means host to network short
    udp_header->dest = htons(dst_port);     //if spec value < 255(8bit) , there is no need to use htons
    udp_header->len = htons(udphdr_size + payload_size);
    udp_header->check = 0;                  //之后需要计算校验和



    //借助伪头部计算 udp 校验和
    //伪头部的大小：伪首部（8个字节）+udp首部（20个字节）+数据
    const int psize = pseudohdr_size + udphdr_size + payload_size;
    char *pseudogram = malloc(psize);
    bzero(pseudogram, sizeof(pseudogram));

    struct pseudo_header *psh = (struct pseudo_header *)pseudogram;
    psh->source_address = src_inet_addr;
    psh->dest_address = dst_inet_addr;
    psh->placeholder = 0;
    psh->protocol = IPPROTO_UDP;
    psh->udp_or_tcp_length = htons(udphdr_size + payload_size);

    memcpy(pseudogram + pseudohdr_size, udp_header, udphdr_size + payload_size);

    //计算 udp 校验和
    udp_header->check = calculate_checkcsum((unsigned short *) pseudogram, psize);

    //↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑至此，数据包构造完毕↑↑↑↑↑↑↑↑↑↑↑↑


    // 创建一个可以构造首部数据的 raw_socket
    int send_socket = -1;
    int one = 1;
    const int *val = &one;
    if ((send_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        return -1;
    }
    if (setsockopt(send_socket, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        return -1;
    }

    // 根据指定 ip 和端口，发送UDP包
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dst_port);
    dest.sin_addr.s_addr = dst_inet_addr;

    ssize_t bytes_sent = sendto(
            send_socket, datagram, datagram_size,
            0, (struct sockaddr *) &dest, sizeof(dest)
    );

    if (bytes_sent < 0) {
        fprintf(stderr, "ERROR: Send datagram fail\n");
    } else {
        printf("%zu bytes send success !\n", bytes_sent);
    }

    // sockfd should close here !
    return 0;
}
