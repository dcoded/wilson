#pragma once
#include "transmission.h"

struct tcp_header : public transmission {
    uint16_t source;
    uint16_t dest;

    uint32_t seq;
    uint32_t ack;

    uint16_t flags;
    uint16_t size;

    uint16_t checksum;
    uint16_t urgent;
};