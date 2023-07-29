/*
(C) tblaha 2023
 */

#define PI_MODE PI_RXTX
#define PI_MSG_IMU_MODE PI_MSG_RXTX

#include <stdio.h>
#include "pi-protocol.h"
#include "pi-messages.h"

void dummyWriter(uint8_t byte)
{
    printf("%02X ", byte);
#if (PI_MODE & PI_RX)
    piParse(byte);
#endif
}

char buf[PI_MAX_PACKET_LEN];

#define N 0xFF
int main(int argc, char** argv) {

    int i = 0;
    while (i < N) {
        printf("Run %02d: TX: ", ++i);
        int len = pi_msg_IMU_pack(buf, i, i+2.f,0,0, 0,0,0);
        piSerialWrite(&dummyWriter, buf, len);
#if PI_MODE & PI_RX
#if PI_MSG_IMU_MODE & PI_MSG_RX
        printf("- RX: msg_id %02hhX, time: %d, R: %f, P: %f, Y: %f, x: %f, y: %f, z: %f.",
            PI_MSG_IMU_ID,
            piMsgImu->time_ms,
            piMsgImu->roll,
            piMsgImu->pitch,
            piMsgImu->yaw,
            piMsgImu->x,
            piMsgImu->y,
            piMsgImu->z
            );
#endif
#endif
        printf("\n");
    }

    return 0;
}
