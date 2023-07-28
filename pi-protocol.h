/*
(C) tblaha 2023
 */

//#pragma once
#ifndef PI_PROTOCOL_H
#define PI_PROTOCOL_H

#include <string.h>
#include <stdint.h>

// --- Protocol definition ---
// message format: | STX | MSG_ID | PAYLOAD_1 | PAYLOAD_2 | ... | PAYLOAD_N |
// if STX occurs in PAYLOAD or MSG_ID, it is escaped by the sequency PI_ESC PI_STX_ESC
// if ESC occurs in PAYLOAD or MSG_ID, it is escaped by the sequence PI_ESC PI_ESC_ESC
#define PI_STX 0xFE
#define PI_ESC 0x01
#define PI_STX_ESC 0x02
#define PI_ESC_ESC 0x03
#define PI_MAX_PAYLOAD_LEN 63

// lenghts
#define PI_ID_LEN 1
#define PI_MAX_PACKET_LEN (PI_MAX_PAYLOAD_LEN + PI_ID_LEN)

// --- utilities ---
#define PI_PAYLOAD_TO_BUFFER(_BUFFER, _PAYLOAD, _LEN) \
    memcpy((_BUFFER)+PI_ID_LEN, (const uint8_t *)( &(_PAYLOAD)), _LEN )

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

#endif // PI_PROTOCOL_H
