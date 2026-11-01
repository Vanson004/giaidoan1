#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "Protocol.h"
#include "nlohmann/json.hpp"
#pragma comment(lib, "Ws2_32.lib")

namespace KetNoi {
    bool receiveData(SOCKET s, char* buf, int doDai);
    bool sendPacket(SOCKET s, uint32_t action, const std::string& jsonStr);
    bool receivePacket(SOCKET s, uint32_t& action, std::string& jsonStr);
    std::string toUtf8(const CString& cstr);
    CString toCString(const std::string& utf8Str);
    std::string getTime();
}
