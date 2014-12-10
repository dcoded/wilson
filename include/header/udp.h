#pragma once
#include "transmission.h"

struct udp_header : public transmission {
    uint16_t source;
    uint16_t dest;
    uint16_t size;
    uint16_t checksum;
};