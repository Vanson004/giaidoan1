#include "Security.h"
#include "md5.h"

using namespace std;

string Security::hashPassword(const string& rawPass) {
    if (rawPass.empty()) return "";
    MD5 md5;
    return md5(rawPass);
}

