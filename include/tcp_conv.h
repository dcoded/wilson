#pragma once
#include <netinet/tcp.h>

#include <tcp_mote.h>

message tcp_to_lite (const uint8_t* data, size_t len)
{

}

int lite_to_tcp (uint8_t* data, const message msg)
{
    return 0;
}