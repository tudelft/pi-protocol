/*
(C) tblaha 2023
 */

//#pragma once
#ifndef PI_MESSAGES_H
#define PI_MESSAGES_H

#include <stdint.h>
#include "pi-protocol.h"

//--------------------------
// ------ MESSAGES ---------
//--------------------------

// ------ IMU ------
#define PI_MSG_IMU_ID 1
#define PI_MSG_IMU_PAYLOAD_LEN (7*4)

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

// packing helper
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

#endif // PI_MESSAGES_H
