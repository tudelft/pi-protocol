/*
(C) tblaha 2023
 */

//#pragma once
#ifndef PI_PROTOCOL_H
#define PI_PROTOCOL_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// --- Protocol definition ---
// message format: | STX | MSG_ID | PAYLOAD_1 | PAYLOAD_2 | ... | PAYLOAD_N |
// if STX occurs in PAYLOAD or MSG_ID, it is escaped by the sequency PI_ESC PI_STX_ESC
// if ESC occurs in PAYLOAD or MSG_ID, it is escaped by the sequence PI_ESC PI_ESC_ESC
#define PI_STX 0xFE
#define PI_ESC 0x01
#define PI_STX_ESC 0x02
#define PI_ESC_ESC 0x03
#define PI_MSG_MAX_PAYLOAD_LEN 0x100
#define PI_ID_LEN 1
#define PI_MAX_PACKET_LEN (PI_MSG_MAX_PAYLOAD_LEN + PI_ID_LEN)

// --- global mode ---
#define PI_TX   1 << 0
#define PI_RX   1 << 1
#define PI_RXTX (PI_TX | PI_RX)

#ifndef PI_MODE
#error "Define PI_MODE as PI_TX, PI_RX or PI_RXTX"
#endif

// --- Message flags ---
#define PI_MSG_NONE_ID 0xFF
#define PI_MSG_MAX_ID 0xFE

#define PI_MSG_NONE 0
#define PI_MSG_TX   1 << 0
#define PI_MSG_RX   1 << 1
#define PI_MSG_RXTX (PI_MSG_TX | PI_MSG_RX)

typedef enum {
    PI_MSG_RX_STATE_A,
    PI_MSG_RX_STATE_B,
    PI_MSG_RX_STATE_NONE
} pi_msg_rx_state_t;

// --- utilities ---
#define PI_PAYLOAD_TO_BUFFER(_BUFFER, _PAYLOAD, _LEN) \
    memcpy((_BUFFER)+PI_ID_LEN, (const uint8_t *)( &(_PAYLOAD)), _LEN )

#include "pi-messages.h"

// --- serializer ---
static inline void piSerialWrite(void (*serialWriter)(uint8_t byte), uint8_t * buf, uint16_t length)
{
    serialWriter(PI_STX);
    for (int i = 0; i < length; i++)
        switch(buf[i]) {
            case PI_STX:
                serialWriter(PI_ESC);
                serialWriter(PI_STX_ESC);
                break;
            case PI_ESC:
                serialWriter(PI_ESC);
                serialWriter(PI_ESC_ESC);
                break;
            default:
                serialWriter(buf[i]);
        }
}

// --- parser ---
#if (PI_MODE & PI_RX)

typedef enum {
    PI_IDLE,
    PI_STX_FOUND,
    PI_ID_FOUND
} pi_parse_state_t;

static inline void piParse(uint8_t byte) {
    static bool piEscHit = false;
    static pi_parse_state_t piState = PI_IDLE;
    static uint8_t msgId = PI_MSG_NONE_ID;
    static uint8_t byteCount = 0;

    if (byte == PI_STX) {
        piEscHit = false;
        piState = PI_STX_FOUND;
        return;
    }

    if (piState == PI_IDLE)
        return;

    if (piEscHit) {
        piEscHit = false;
        switch(byte) {
            case PI_STX_ESC:
                byte = PI_STX;
                break;
            case PI_ESC_ESC:
                byte = PI_ESC;
                break;
            default:
                // failure, next byte MUST have been PI_STX_ESC or PI_ESC_ESC
                piState = PI_IDLE;
                return;
        }
    } else {
        if (byte == PI_ESC) {
            piEscHit = true;
            return;
        }
    }

    switch(piState) {
        case PI_STX_FOUND:
            // parse id
            msgId = byte;
            piState = PI_ID_FOUND;
            byteCount = 0;
            break;
        case PI_ID_FOUND:
            // payload time
            uint8_t error = piParseMsg(msgId, byte, byteCount++);
            if (error) {
                printf("\n piParseError at id 0x%02hhX, byte 0x%02hhX: %d\n", msgId, byte, error);
                msgId = PI_MSG_NONE_ID;
                piState = PI_IDLE;
            }
            break;
    }
};

#endif



#endif // PI_PROTOCOL_H
