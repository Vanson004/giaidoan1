#pragma once
#include <string>

using namespace std;

class Security {
public:
    static string hashPassword(const string& rawPass);
};

