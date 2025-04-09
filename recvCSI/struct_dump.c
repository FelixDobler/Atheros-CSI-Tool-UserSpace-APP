/*
Helper used to find out the struct bit alignment
*/

#include <stdbool.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define KERNEL_CSI_ST_LEN 23

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

int main(int argc, char *argv[])
{
    FILE *fp;
    fp = fopen(argv[1], "w");
    if (!fp)
    {
        printf("Fail to open <output_file>\n");
        fclose(fp);
        return 0;
    }

    csi_struct csi_status = {
        .tstamp = ~0,
        .channel = ~0,
        .chanBW = ~0,
        .rate = ~0,
        .nr = ~0,
        .nc = ~0,
        .num_tones = ~0,
        .noise = ~0,
        .phyerr = ~0,
        .rssi = ~0,
        .rssi_0 = ~0,
        .rssi_1 = ~0,
        .rssi_2 = ~0,
        .payload_len = ~0,
        .csi_len = ~0,
        .buf_len = ~0};

    fwrite(&csi_status, sizeof(csi_struct), 1, fp);
    fclose(fp);
}
