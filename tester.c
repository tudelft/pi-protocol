/*
(C) tblaha 2023
 */

#include <stdio.h>
#include "pi-protocol.h"
#include "pi-messages.h"

void dummyWriter(uint8_t byte)
{
    printf("%02X ", byte);
}

char buf[PI_MAX_PACKET_LEN];

#define N 10
int main(int argc, char** argv) {

    int i = N;
    while (i > 0) {
        printf("%02d: ", i--);
        int len = pi_msg_IMU_pack(buf, i, 0,0,0, 0,0,0);
        piSerialWrite(&dummyWriter, buf, len);
        printf("\n");
    }

    return 0;
}
