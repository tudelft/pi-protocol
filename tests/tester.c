/*
(C) tblaha 2023
 */

#include <stdio.h>
#include "../pi-protocol.h"
#include "../pi-messages.h"

#define PI_DEBUG

void dummyWriter(uint8_t byte)
{
    printf("%02X ", byte); // implement your serialWrite function here
#if (PI_MODE & PI_RX)
    piParse(byte);
#endif
}

uint8_t buf[PI_MAX_PACKET_LEN];

#define N 0x0F
int main(int argc, char** argv) {

    int i = 0;
    while (i <= N) {
        printf("Run %02d, msg IMU: TX: ", ++i);
        int len = piMsgImuPack(buf, i, i+2.f,0,0, 0,0,0);
        piSerialWrite(&dummyWriter, buf, len);

        printf("\nRun %02d, msg DUMMY_MESSAGE: TX: ", i);
        len = piMsgDummyMessagePack(buf, i, i+2.f,0);
        piSerialWrite(&dummyWriter, buf, len);

#if (PI_MODE & PI_RX) && (PI_MSG_IMU_MODE & PI_MSG_RX)
        // this if check should be equivalent to checking if piMsgImu is a NULL pointer
        // it is absolutely required. Otherwise there will be SegFaults if a message is read before it was received for the first time.
        if (piMsgImuRxState < PI_MSG_RX_STATE_NONE) {
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
        }
#endif
        printf("\n");
    }

    return 0;
}
