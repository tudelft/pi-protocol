
/*
 * Copyright 2024 Till Blaha (Delft University of Technology)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "pi-protocol.h"
#include "pi-messages.h"

// well known slow (without lookup tables) crc8 code taken from betaflight
uint8_t crc8_calc(uint8_t crc, unsigned char a, uint8_t poly)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ poly;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

//uint8_t crc8_update(uint8_t crc, const void *data, uint32_t length, uint8_t poly)
//{
//    const uint8_t *p = (const uint8_t *)data;
//    const uint8_t *pend = p + length;
//
//    for (; p != pend; p++) {
//        crc = crc8_calc(crc, *p, poly);
//    }
//    return crc;
//}

// --- sender --- //
#if (PI_MODE & PI_TX)
void piSendMsg(void * msg_raw, void (*serialWriter)(uint8_t byte)) {
    uint8_t buf[2*PI_MAX_PACKET_LEN]; // wasting some stack
    unsigned int num_bytes = piAccumulateMsg( msg_raw, buf );

    for (unsigned int i = 0; i < num_bytes; i++) {
        serialWriter(buf[i]);
    }
}
unsigned int piAccumulateMsg(void * msg_raw, uint8_t * buf) {
    // cast to uint8_t pointer
    uint8_t * msg = (uint8_t *) msg_raw;

    unsigned int i = 0;

    // start byte first. Next field (id) will be handled with the usual escape
    buf[i++] = PI_STX;

    // this is not that nice, because it assume that id and len are 1 byte
    bool id_sent = false;
    uint8_t id = *(msg++);
    uint8_t len = *(msg++) + 1 + 1; // one extra for id, one for checksum
    uint8_t checksum = crc8_calc(0, id, PI_CRC8_POLYNOMIAL);

    while (len-- > 0) {
        uint8_t byte;
        if (!id_sent) {
            // send id
            byte = id;
            id_sent = true;
        } else if (len > 0) {
            // send message payload
            byte = *(msg++);
            checksum = crc8_calc(checksum, byte, PI_CRC8_POLYNOMIAL);
        } else {
            // send checksum byte
            byte = checksum;
        }
        switch(byte) {
            case PI_STX:
                buf[i++] = PI_ESC;
                buf[i++] = PI_STX_ESC;
                break;
            case PI_ESC:
                buf[i++] = PI_ESC;
                buf[i++] = PI_ESC_ESC;
                break;
            default:
                buf[i++] = byte;
        }
    }
    return i;
}
#endif // #if (PI_MODE & PI_TX)

// --- parser ---
#if (PI_MODE & PI_RX)
__attribute__((unused)) uint8_t piParse(pi_parse_states_t * p, uint8_t byte) {
    uint8_t res = PI_MSG_NONE_ID;

    //p->piEscHit = false;
    //p->piState = PI_IDLE;
    //p->msgId = PI_MSG_NONE_ID;
    //p->byteCount = 0;
    p->msgParseResult = PI_PARSE_MSG_NO_ERROR;

#ifdef PI_STATS
    piStats[PI_PARSE_INVOKE]++;
#endif

    if (byte == PI_STX) {
#ifdef PI_STATS
        piStats[PI_STX_COUNT]++;
#endif
        p->piEscHit = false;
        p->piState = PI_STX_FOUND;
        return res;
    }

    if (p->piState == PI_IDLE)
        return res;

    if (p->piEscHit) {
        p->piEscHit = false;
        switch(byte) {
            case PI_STX_ESC:
                byte = PI_STX;
                break;
            case PI_ESC_ESC:
                byte = PI_ESC;
                break;
            default:
                // failure, next byte MUST have been PI_STX_ESC or PI_ESC_ESC
#ifdef PI_DEBUG
                printf("Escaping error: next byte was neither PI_STX_ESC nor PI_ESC_ESC, but rather %02hhX\n", byte);
#endif
#ifdef PI_STATS
                piStats[PI_ESC_ERROR]++;
#endif
                p->piState = PI_IDLE;
                return res;
        }
    } else {
        if (byte == PI_ESC) {
            p->piEscHit = true;
            return res;
        }
    }


    switch(p->piState) {
        case PI_IDLE: // can never happen, so fall through to satisfy GCC -Wall
            break;
        case PI_STX_FOUND:
            // parse id
            p->msgId = byte;
            p->checksum = crc8_calc(0, byte, PI_CRC8_POLYNOMIAL);
            p->piState = PI_ID_FOUND;
            p->byteCount = 0;
            break;
        case PI_ID_FOUND:
            // payload time
            p->msgParseResult = piParseIntoMsg(p, byte);
            (p->byteCount)++;
            if (p->msgParseResult == PI_PARSE_MSG_SUCCESS) 
            {
                res = p->msgId;
#ifdef PI_STATS
                piStats[PI_SUCCESS]++;
#endif
            } 
            if (p->msgParseResult > PI_PARSE_MSG_SUCCESS) {
#ifdef PI_DEBUG
                printf("\n msgParseResult > PI_PARSE_MSG_SUCCESS at id 0x%02hhX, byte 0x%02hhX: %d\n", p->msgId, byte, p->msgParseResult);
#endif
                p->msgId = PI_MSG_NONE_ID;
                p->piState = PI_IDLE;
#ifdef PI_STATS
                piStats[p->msgParseResult]++;
#endif
            }
            break;
    }
    return res;
}

#ifdef PI_STATS
unsigned int piStats[NUM_PI_STATS_RESULT] = { 0,0,0,0, 0,0,0, 0,0,0 };

__attribute__((unused)) void piPrintStats(int (*printer)(const char * s, ...)) {
    static int i = 0;
    printer("\n+------------ piPrintStats invokation %d -------------+\n", i++);
    printer("|%-30s|%-10s|\n", "Result", "Occurence");
    printer("|------------------------------|----------|\n");
    for (int j=0; j < NUM_PI_STATS_RESULT; j++)
        printer("|%30s|%10d|\n", piStatsNames[j], piStats[j]);
    printer("|------------------------------|----------|\n");
}
#endif// #ifdef PI_STATS

#endif// #if (PI_MODE & PI_RX)


#ifdef __cplusplus
}
#endif
