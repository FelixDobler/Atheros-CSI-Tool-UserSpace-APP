/*
 * =====================================================================================
 *       Filename:  sendData.c
 *
 *    Description:  send packets
 *        Version:  1.0
 *
 *         Author:  Yaxiong Xie
 *         Email :  <xieyaxiongfly@gmail.com>
 *   Organization:  WANS group @ Nanyang Technological University
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * =====================================================================================
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

/* Define the defult destination MAC address */
#define DEFAULT_DEST_MAC0 0xB4
#define DEFAULT_DEST_MAC1 0x82
#define DEFAULT_DEST_MAC2 0xC5
#define DEFAULT_DEST_MAC3 0x58
#define DEFAULT_DEST_MAC4 0x6b
#define DEFAULT_DEST_MAC5 0x20

#define DEFAULT_IF "wlan1"
#define BUF_SIZ 2048

static volatile sig_atomic_t keep_running = 1;

static void stop_handler(){
    keep_running = 0;
}

int main(int argc, char *argv[]) {
    int sockfd;
    int i;
    struct ifreq if_idx;
    struct ifreq if_mac;
    int tx_len = 0, Cnt, cntUnlimited = 0;
    char sendbuf[BUF_SIZ];
    unsigned int DstAddr[6];
    struct ether_header *eh = (struct ether_header *)sendbuf;
    struct iphdr *iph = (struct iphdr *)(sendbuf + sizeof(struct ether_header));
    struct sockaddr_ll socket_address;
    char ifName[IFNAMSIZ];

    if (argc == 1) {
        printf("Usage:   %s ifName DstMacAddr NumOfPacketToSend\n", argv[0]);
        printf("Example: %s wlan0 00:7F:5D:3E:4A 100\n", argv[0]);
        printf("Set NumOfPacketToSend to -1 for unlimited\n");
        exit(0);
    }

    /* Get interface name */
    if (argc > 1)
        strcpy(ifName, argv[1]);
    else
        strcpy(ifName, DEFAULT_IF);

    // dst address seperated by :, example: 00:7F:5D:3E:4A
    if (argc > 2) {
        sscanf(argv[2], "%x:%x:%x:%x:%x:%x", &DstAddr[0], &DstAddr[1], &DstAddr[2], &DstAddr[3], &DstAddr[4], &DstAddr[5]);
        // printf("DstMacAddr: %02x:%02x:%02x:%02x:%02x:%02x\n",DstAddr[0],DstAddr[1],DstAddr[2],DstAddr[3],DstAddr[4],DstAddr[5]);
    } else {
        DstAddr[0] = DEFAULT_DEST_MAC0;
        DstAddr[1] = DEFAULT_DEST_MAC1;
        DstAddr[2] = DEFAULT_DEST_MAC2;
        DstAddr[3] = DEFAULT_DEST_MAC3;
        DstAddr[4] = DEFAULT_DEST_MAC4;
        DstAddr[5] = DEFAULT_DEST_MAC5;
    }

    if (argc > 3) {
        Cnt = atoi(argv[3]);
        if (Cnt == -1) {
            cntUnlimited = 1;
        }
    } else {
        Cnt = 1;
    }
    /* Open RAW socket to send on */
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
        perror("socket");
    }

    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifName, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");
    /* Get the MAC address of the interface to send on */
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifName, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");

    /* Construct the Ethernet header */
    memset(sendbuf, 0, BUF_SIZ);
    /* Ethernet header */
    eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
    eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
    eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
    eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
    eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
    eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
    eh->ether_dhost[0] = DstAddr[0];
    eh->ether_dhost[1] = DstAddr[1];
    eh->ether_dhost[2] = DstAddr[2];
    eh->ether_dhost[3] = DstAddr[3];
    eh->ether_dhost[4] = DstAddr[4];
    eh->ether_dhost[5] = DstAddr[5];

    /* Ethertype field */
    eh->ether_type = htons(ETH_P_IP);
    tx_len += sizeof(struct ether_header);

    /* Packet data
     * We just set it to 0xaa you send arbitrary payload you like*/
    for (i = 1; i <= 1000; i++) {
        sendbuf[tx_len++] = 0xaa;
    }
    printf("Packet Length is: %d,pkt_num is: %d\n", tx_len, Cnt);

    /* Index of the network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* RAW communication*/
    socket_address.sll_family = PF_PACKET;
    /* we don't use a protocoll above ethernet layer
     *   ->just use anything here*/
    socket_address.sll_protocol = htons(ETH_P_IP);

    /* ARP hardware identifier is ethernet*/
    socket_address.sll_hatype = ARPHRD_ETHER;

    /* target is another host*/
    socket_address.sll_pkttype = PACKET_OTHERHOST;

    /* address length*/
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    socket_address.sll_addr[0] = DstAddr[0];
    socket_address.sll_addr[1] = DstAddr[1];
    socket_address.sll_addr[2] = DstAddr[2];
    socket_address.sll_addr[3] = DstAddr[3];
    socket_address.sll_addr[4] = DstAddr[4];
    socket_address.sll_addr[5] = DstAddr[5];

    struct sched_param param;
    param.__sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler error");
    }

    int policy = sched_getscheduler(0);
    printf("New scheduling policy: %s\n",
           (policy == SCHED_FIFO)    ? "FIFO"
           : (policy == SCHED_RR)    ? "Round Robin"
           : (policy == SCHED_OTHER) ? "Other"
                                     : "Unknown");

    signal(SIGINT, stop_handler);

    /* Send packets */
    i = 1;
    while (keep_running && (cntUnlimited || i <= Cnt)) {
        /* you set the time interval between two transmitting packets
         * for example, here we set it to 50 microseconds
         * set to 0 if you don't need it
         */

        struct timespec sleep_ts;
        sleep_ts.tv_sec = 0;
        sleep_ts.tv_nsec = 1e6;

        struct timespec return_ts;

        if (nanosleep(&sleep_ts, &return_ts) == -1) {
            printf("sleep failed\n");
        }
        if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
            printf("Send failed\n");
        }
        // get the time using clock_gettime
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        // printf("%d, %ld\n", i, ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
        i++;
    }

    // printf("SendData: Exiting now\n");
    fflush(stdout);
    return 0;
}
