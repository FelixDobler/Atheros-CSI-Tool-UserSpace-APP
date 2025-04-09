/*
 * =====================================================================================
 *       Filename:  main.c
 *
 *    Description:  Here is an example for receiving CSI matrix
 *                  Basic CSi procesing fucntion is also implemented and called
 *                  Check csi_fun.c for detail of the processing function
 *        Version:  1.0
 *
 *         Author:  Yaxiong Xie
 *         Email :  <xieyaxiongfly@gmail.com>
 *   Organization:  WANDS group @ Nanyang Technological University
 *
 *   Copyright (c)  WANDS group @ Nanyang Technological University
 * =====================================================================================
 */
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include "csi_fun.h"

#define BUFSIZE 4096

static volatile sig_atomic_t keep_running;
unsigned char buf_addr[BUFSIZE];
unsigned char data_buf[1500];

COMPLEX csi_matrix[3][3][114];
csi_struct* csi_status;

static void sig_handler(int signo) {
    // if (signo == SIGINT || signo == SIGTERM)
    keep_running = 0;
}

int main(int argc, char* argv[]) {
    FILE* fp;
    int fd;
    int i;
    int msg_cnt, cnt;
    int log_flag;
    unsigned char endian_flag;
    u_int16_t buf_len;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    log_flag = 1;
    csi_status = (csi_struct*)malloc(sizeof(csi_struct));
    /* check usage */
    if (1 == argc) {
        /* If you want to log the CSI for off-line processing,
         * you need to specify the name of the output file
         */
        log_flag = 0;
        printf("/**************************************/\n");
        printf("/*   Usage: recv_csi <output_file>    */\n");

        printf("/*   csi struct size: %lu              */\n", sizeof(csi_struct));
        printf("/*   csi matrix size: %lu            */\n", sizeof(csi_matrix));
        printf("/**************************************/\n");
    }
    if (2 == argc) {
        fp = fopen(argv[1], "w");
        if (!fp) {
            printf("Fail to open <output_file>, are you root?\n");
            fclose(fp);
            return 0;
        }
    }
    if (argc > 2) {
        printf(" Too many input arguments !\n");
        return 0;
    }

    fd = open_csi_device();
    if (fd < 0) {
        perror("Failed to open the device...");
        return errno;
    }

    // printf("#Receiving data! Press Ctrl+C to quit!\n");

    keep_running = 1;
    msg_cnt = 0;

    while (keep_running == 1) {
        /* keep listening to the kernel and waiting for the csi report */
        cnt = read_csi_buf(buf_addr, fd, BUFSIZE);

        if (cnt) {
            /* fill the status struct with information about the rx packet */
            record_status(buf_addr, cnt, csi_status);

            /*
             * fill the payload buffer with the payload
             * fill the CSI matrix with the extracted CSI value
             */
            record_csi_payload(buf_addr, csi_status, data_buf, csi_matrix);

            /* Till now, we store the packet status in the struct csi_status
             * store the packet payload in the data buffer
             * store the csi matrix in the csi buffer
             * with all those data, we can build our own processing function!
             */
            process_csi(data_buf, csi_status, csi_matrix);

            // printf("Recv %dth msg with rate: 0x%02x | payload len: %d | on channel: %d\n",total_msg_cnt,csi_status->rate,csi_status->payload_len, csi_status->channel);
            // printf(".");

            int payload_len = csi_status->payload_len;
            // unsigned char payload[payload_len];

            // int start_pos = -1;
            // int end_pos = -1;

            // for (int i = 0; i < payload_len; i++) {
            //     if (data_buf[i] == 0xaa) {
            //         if (start_pos == -1) {
            //             start_pos = i;
            //             end_pos = i;
            //         } else {
            //             end_pos = i;
            //         }
            //     } else {
            //         if (start_pos != -1 && end_pos != -1) {
            //             printf("Segment from %d to %d, length %d", start_pos, end_pos, end_pos - start_pos + 1);
            //         }
            //         start_pos = -1;
            //     }
            // }

            if (payload_len < 1035) {
                continue;
            }
            for (i = 36; i < 1036; i++) {
                if (data_buf[i] != 0xaa) {
                    continue;
                }
            }

            msg_cnt += 1;

            printf("Recv %d msg at %ld\n", msg_cnt, csi_status->tstamp);
            // printf("%d", csi_status->payload_len);

            // memcpy(payload, data_buf, payload_len);
            // fwrite(payload, payload_len, 1, stdout);

            /* log the received data for off-line processing */

            if (log_flag) {
                // buf_len = csi_status->buf_len;
                // fwrite(&buf_len,1,2,fp);
                // fwrite(buf_addr,1,buf_len,fp);
                store_csi(csi_status, csi_matrix, fp);
            }
        }
    }

    printf("\n");
    fflush(stdout);
    fclose(fp);
    close_csi_device(fd);
    free(csi_status);
    
    exit(0);
}
