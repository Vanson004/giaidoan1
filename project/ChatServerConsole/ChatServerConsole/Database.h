#pragma once
#include <string>
#include <vector>

using namespace std;

struct HistoryMessage {
    string sender;
    string receiver;
    string content;
    string timestamp;
    int isRead;
};

class Database {
public:
    static bool initDB();
    static bool registerUser(const string& username, const string& password);
    static bool loginUser(const string& username, const string& password);
    static bool saveMessage(const string& sender, const string& receiver, const string& content, int isRead = 0);
    static int updateMessagesIsRead(const string& reader, const string& sender);
    static vector<HistoryMessage> getChatHistory(const string& user1, const string& user2);
    static vector<string> getAllUsers();
};

