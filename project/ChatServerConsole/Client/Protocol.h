#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct PacketHeader {
    uint32_t action;
    uint32_t dataSize;
};
#pragma pack(pop)
#define ACTION_REGISTER 1u
#define ACTION_LOGIN  2u
#define ACTION_CHAT   3u
#define ACTION_USER_STATUS 4u
#define ACTION_GET_HISTORY 5u
#define ACTION_MSG_READ    6u
#define RES_SUCCESS "SUCCESS"
#define RES_FAIL   "FAIL"
#define SERVER_DEFAULT_IP  "127.0.0.1"
#define SERVER_DEFAULT_PORT 9999


