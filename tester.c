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

#define N 10 

pi_message_t msg;
char buf[PI_MAX_PACKET_LEN];

int main(int argc, char** argv) {

    int i = N;
    while (i > 0) {
        printf("%02d: ", i--);
        int len = pi_msg_IMU_pack(buf, i, 0,0,0, 0,0,0);
        //PI_MSG_TO_SEND_BUFFER(buf, msg);
        piSerialWrite(&dummyWriter, buf, len);
        printf("\n");
    }

    return 0;
}
