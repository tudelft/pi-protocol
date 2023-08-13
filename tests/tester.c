/*
(C) tblaha 2023
 */

#include <stdio.h>
#include "../src/pi-protocol.h"
#include "../src/pi-messages.h"

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
    while (i++ <= N) {
#if (PI_MODE & PI_TX) && (PI_MSG_IMU_MODE & PI_TX)
        printf("Run %02d, msg IMU: TX: ", i);
        piMsgImuTx.time_ms = i;
        piMsgImuTx.roll = i+2.f;
        piMsgImuTx.pitch = 0.;
        piMsgImuTx.yaw = 0.;
        piMsgImuTx.x = 0.;
        piMsgImuTx.y = 0.;
        piMsgImuTx.z = 0.;
        piSendMsg(&piMsgImuTx, &dummyWriter);
#endif

#if (PI_MODE & PI_TX) && (PI_MSG_DUMMY_MESSAGE_MODE & PI_TX)
        printf("\nRun %02d, msg DUMMY_MESSAGE: TX: ", i);
        piMsgDummyMessageTx.time_ms = i;
        piMsgDummyMessageTx.roll = 0.;
        piMsgDummyMessageTx.roll1 = 0.;
        piSendMsg(&piMsgDummyMessageTx, &dummyWriter);
#endif

#if (PI_MODE & PI_RX) && (PI_MSG_IMU_MODE & PI_RX)
        // this if check should be equivalent to checking if piMsgImu is a NULL pointer
        // it is absolutely required. Otherwise there will be SegFaults if a message is read before it was received for the first time.
        if (piMsgImuRxState < PI_MSG_RX_STATE_NONE) {
            printf("- RX: msg_id %02hhX, Rx Buffer: %d, time: %d, R: %f, P: %f, Y: %f, x: %f, y: %f, z: %f.",
                PI_MSG_IMU_ID,
                piMsgImuRxState,
                piMsgImuB.time_ms,
                piMsgImuB.roll,
                piMsgImuB.pitch,
                piMsgImuB.yaw,
                piMsgImuB.x,
                piMsgImuB.y,
                piMsgImuB.z
                );
        }
#endif
#if (PI_MODE & PI_RX)
#ifdef PI_USE_PRINT_MSGS
        piPrintMsgs(&printf);
#endif
#endif
        printf("\n");
    }

#if (PI_MODE & PI_RX)
    // ---- Parse failure conditions ---- //

    // failure with PI_INVALID_ID:
    uint8_t invalid_msg_id[10] = {0xFE, 0xFF, 0x00, 0x01, 0x02};
    for (int i = 0; i < 10; i++) {
        piParse(invalid_msg_id[i]);
    }

    // STX found, but neither success nor failure:
    uint8_t multiple_stx[10] = {0xFE, 0x02, 0x00, 0xFE, 0x01, 0x03};
    for (int i = 0; i < 10; i++) {
        piParse(multiple_stx[i]);
    }

    // 2x escaping error, 1x STX found, 1x NO such message
    uint8_t escaping[10] = {0xFE, 0x01, 0x00, 0x01, 0x04, 0xFE, 0xF0};
    for (int i = 0; i < 10; i++) {
        piParse(escaping[i]);
    }

    // 1x exceed msg length, 2x STX found
    uint8_t exceed_msg_len[PI_MSG_IMU_PAYLOAD_LEN+5] = {
        [0] = 0xFE,
        [1] = 0x01,
        [2] = 0x03,
        [PI_MSG_IMU_PAYLOAD_LEN+3] = 0x00,
        [PI_MSG_IMU_PAYLOAD_LEN+4] = 0xFE
        };
    for (int i = 0; i < PI_MSG_IMU_PAYLOAD_LEN+5; i++) {
        piParse(exceed_msg_len[i]);
    }

    // cannot test exceed max lenth at this point
    // NULL buffer error can also not be tested, as it should never happen

#ifdef PI_STATS
        piPrintStats(&printf);
        // expected output:
/*
+------------ piPrintStats invokation 0 -------------+
|Result                        |Occurence |
|------------------------------|----------|
|                    PI_SUCCESS|        32|
|                 PI_INVALID_ID|         1|
|    PI_EXCEEDS_MAX_PAYLOAD_LEN|         0|
|    PI_EXCEEDS_MSG_PAYLOAD_LEN|         1|
|                PI_NULL_BUFFER|         0|
|                PI_NO_SUCH_MSG|         1|
|                  PI_ESC_ERROR|         1|
|                  PI_STX_COUNT|        39|
|               PI_PARSE_INVOKE|       913|
|------------------------------|----------|
*/
#endif // ifdef PI_STATS

#endif // if PI_MODE & PI_RX

    return 0;
}
