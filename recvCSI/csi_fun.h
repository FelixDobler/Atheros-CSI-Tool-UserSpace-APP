/*
 * =====================================================================================
 *       Filename:  csi_fun.h
 *
 *    Description:  head file for csi processing fucntion
 *        Version:  1.0
 *
 *         Author:  Yaxiong Xie
 *         Email :  <xieyaxiongfly@gmail.com>
 *   Organization:  WANDS group @ Nanyang Technological University
 *
 *   Copyright (c)  WANDS group @ Nanyang Technological University
 * =====================================================================================
 */
#include <stdbool.h>
#include <sys/types.h>

#define KERNEL_CSI_ST_LEN 23
typedef struct
{
    int real;
    int imag;
} COMPLEX;

typedef struct
{
    u_int64_t tstamp; /* h/w assigned time stamp */

    u_int16_t channel; /* wireless channel (represented in Hz)*/
    u_int8_t chanBW;   /* channel bandwidth (0->20MHz,1->40MHz)*/

    u_int8_t rate;      /* transmission rate*/
    u_int8_t nr;        /* number of receiving antennas*/
    u_int8_t nc;        /* number of transmitting antennas*/
    u_int8_t num_tones; /* number of tones (subcarriers) */
    u_int8_t noise;     /* noise floor (to be updated)*/

    u_int8_t phyerr; /* phy error code (set to 0 if correct)*/

    u_int8_t rssi;   /*  rx frame RSSI */
    u_int8_t rssi_0; /*  rx frame RSSI [ctl, chain 0] */
    u_int8_t rssi_1; /*  rx frame RSSI [ctl, chain 1] */
    u_int8_t rssi_2; /*  rx frame RSSI [ctl, chain 2] */

    u_int16_t payload_len; /*  payload length (bytes) */
    u_int16_t csi_len;     /*  csi data length (bytes) */
    u_int16_t buf_len;     /*  data length in buffer */
} csi_struct;

bool is_big_endian();
int open_csi_device();
void close_csi_device(int fd);
int read_csi_buf(unsigned char *buf_addr, int fd, int BUFSIZE);
void record_status(unsigned char *buf_addr, int cnt, csi_struct *csi_status);
void record_csi_payload(unsigned char *buf_addr, csi_struct *csi_status, unsigned char *data_buf, COMPLEX csi_buf[3][3][114]);
void process_csi(unsigned char *data_buf, csi_struct *csi_status, COMPLEX csi_buf[3][3][114]);
void store_csi(csi_struct* csi_status, COMPLEX csi_buf[3][3][114], FILE* fp);
