/*
(C) tblaha 2023
 */

//#pragma once
#ifndef PI_MESSAGES_H
#define PI_MESSAGES_H

#include <stdint.h>

typedef enum {
    PI_PARSE_MSG_SUCCESS,
    PI_PARSE_MSG_INVALID_ID,
    PI_PARSE_MSG_EXCEEDS_MAX_PAYLOAD_LEN,
    PI_PARSE_MSG_EXCEEDS_MSG_PAYLOAD_LEN,
    PI_PARSE_MSG_NULL_BUFFER,
    PI_PARSE_MSG_NO_SUCH_MSG,
} pi_parse_msg_result_t;

//--------------------------
// ------ MESSAGES ---------
//--------------------------

// ------ IMU ------
#if PI_MSG_IMU_MODE

#define PI_MSG_IMU_ID 1 // do not choose 255 (0xFF)!
#define PI_MSG_IMU_PAYLOAD_LEN (7*4)

#ifndef PI_MSG_IMU_MODE
#define PI_MSG_IMU_MODE PI_NONE
#endif

// payload definition
typedef struct __pi_IMU_t
{
    uint32_t time_ms;
    float roll;
    float pitch;
    float yaw;
    float x;
    float y;
    float z;
} __attribute__((packed)) pi_IMU_t;

// TODO: macro to reduce boilerplate
#if (PI_MSG_IMU_MODE & PI_MSG_RX)
pi_IMU_t piMsgImuA;
pi_IMU_t piMsgImuB;
pi_IMU_t* piMsgImu = NULL;
pi_msg_rx_state_t piMsgImuRxState = PI_MSG_RX_STATE_NONE;
#endif

// packing helper
#if (PI_MSG_IMU_MODE & PI_MSG_TX)
static inline int pi_msg_IMU_pack(uint8_t* buf, uint32_t time_ms, float roll, float pitch, float yaw, float x, float y, float z)
{
    buf[0] = PI_MSG_IMU_ID;

    pi_IMU_t payload;
    payload.time_ms = time_ms;
    payload.roll = roll;
    payload.pitch = pitch;
    payload.yaw = yaw;
    payload.x = x;
    payload.y = y;
    payload.z = z;

    PI_PAYLOAD_TO_BUFFER(buf, payload, PI_MSG_IMU_PAYLOAD_LEN);

    return PI_ID_LEN+PI_MSG_IMU_PAYLOAD_LEN;
}
#endif

#endif


// -----
// TODO: how to figure out if thisfunction is even needed at all (PI_MODE_RX set)
static inline uint8_t piParseMsg(const uint8_t msgId, const uint8_t byte, const uint8_t byteCount) {
    static void * piMsgRxBuffer = NULL;

    if (msgId >= PI_MSG_MAX_ID)
        return PI_PARSE_MSG_INVALID_ID;

    if (byteCount >= (PI_MSG_MAX_PAYLOAD_LEN-1))
        return PI_PARSE_MSG_EXCEEDS_MAX_PAYLOAD_LEN;

    switch(msgId) {
#if (PI_MSG_IMU_MODE & PI_MSG_RX)
        case PI_MSG_IMU_ID:
            if (byteCount == 0) {
                piMsgRxBuffer = (piMsgImuRxState > PI_MSG_RX_STATE_A) ? &piMsgImuA : &piMsgImuB;
            } else if (byteCount >= PI_MSG_IMU_PAYLOAD_LEN) {
                return PI_PARSE_MSG_EXCEEDS_MSG_PAYLOAD_LEN;
            }

            // macro or inline func to reduce boilerplate, as this doent depend on anything
            if (piMsgRxBuffer) {
                *(uint8_t *)(piMsgRxBuffer+byteCount) = byte;
            } else {
                return PI_PARSE_MSG_NULL_BUFFER;
            }

            if (byteCount == (PI_MSG_IMU_PAYLOAD_LEN-1)) {
                // message complete! swtich buffers
                piMsgImuRxState = (piMsgImuRxState == PI_MSG_RX_STATE_A) ? PI_MSG_RX_STATE_B : PI_MSG_RX_STATE_A;
                piMsgImu = (piMsgImuRxState == PI_MSG_RX_STATE_A) ? &piMsgImuA : &piMsgImuB;
            }
            break;
#endif
        //case 
        default:
            return PI_PARSE_MSG_NO_SUCH_MSG;
    }
    return PI_PARSE_MSG_SUCCESS;
}

#endif // PI_MESSAGES_H