#include "pch.h"
#include "KetNoi.h"
#include <vector>

using namespace std;

namespace KetNoi {
    bool receiveData(SOCKET s, char* buf, int doDai) {
        int daNhan = 0;
        while (daNhan < doDai) {
            int r = recv(s, buf + daNhan, doDai - daNhan, 0);
            if (r <= 0) return false;
            daNhan += r;
        }
        return true;
    }

    bool sendPacket(SOCKET s, uint32_t action, const string& jsonStr) {
        PacketHeader hdr;
        hdr.action = action;
        hdr.dataSize = (uint32_t)jsonStr.size();
        if (send(s, (const char*)&hdr, sizeof(hdr), 0) == SOCKET_ERROR)
            return false;
        if (hdr.dataSize > 0) {
            if (send(s, jsonStr.c_str(), (int)jsonStr.size(), 0) == SOCKET_ERROR)
                return false;
        }
        return true;
    }

    bool receivePacket(SOCKET s, uint32_t& action, string& jsonStr) {
        PacketHeader hdr;
        if (!receiveData(s, (char*)&hdr, sizeof(hdr))) return false;
        if (hdr.dataSize > 1024u * 1024u) return false;
        action = hdr.action;
        if (hdr.dataSize == 0) {
            jsonStr = "";
            return true;
        }
        vector<char> buf(hdr.dataSize + 1, 0);
        if (!receiveData(s, buf.data(), (int)hdr.dataSize)) return false;
        buf[hdr.dataSize] = '\0';
        jsonStr = string(buf.data(), hdr.dataSize);
        return true;
    }

    string toUtf8(const CString& cstr) {
        if (cstr.IsEmpty()) return "";
        int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)cstr, -1, nullptr, 0, nullptr, nullptr);
        if (len <= 0) return "";
        string res(len, '\0');
        WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)cstr, -1, &res[0], len, nullptr, nullptr);
        res.pop_back();
        return res;
    }

    CString toCString(const string& utf8Str) {
        if (utf8Str.empty()) return CString();
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
        if (len <= 0) return CString();
        CString res;
        wchar_t* p = res.GetBuffer(len);
        MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, p, len);
        res.ReleaseBuffer();
        return res;
    }

    string getTime() {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char buf[32];
        sprintf_s(buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
        return string(buf);
    }
}

