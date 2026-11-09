#include "Database.h"
#include "Security.h"
#include "sqlite3.h"
#include <windows.h>
#include <string>
#include <vector>
using namespace std;
static CRITICAL_SECTION g_csDB;
static bool g_bDbCsInit = false;

class AutoDBLock {
public:
    AutoDBLock() {
        if (!g_bDbCsInit) {
            InitializeCriticalSection(&g_csDB);
            g_bDbCsInit = true;
        }
        EnterCriticalSection(&g_csDB);
    }
    ~AutoDBLock() {
        LeaveCriticalSection(&g_csDB);
    }
};

static string GetDBPath() {
    static string cachedPath;
    if (!cachedPath.empty()) return cachedPath;
    if (GetFileAttributesA("D:\\ProjectCMC\\chat_app.db") != INVALID_FILE_ATTRIBUTES) {
        cachedPath = "D:\\ProjectCMC\\chat_app.db";
        return cachedPath;
    }
    char path[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, path, MAX_PATH);
    string dir(path);
    while (true) {
        size_t pos = dir.find_last_of("\\/");
        if (pos == string::npos) break;
        dir = dir.substr(0, pos);

        string candidate = dir + "\\chat_app.db";
        if (GetFileAttributesA(candidate.c_str()) != INVALID_FILE_ATTRIBUTES) {
            cachedPath = candidate;
            return cachedPath;
        }
    }
    cachedPath = "D:\\ProjectCMC\\chat_app.db";
    return cachedPath;
}

bool Database::initDB() {
    AutoDBLock lock;
    sqlite3* db = nullptr;
    char* errMsg = nullptr;
    string dbPath = GetDBPath();
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    sqlite3_busy_timeout(db, 5000);
    sqlite3_exec(db,"PRAGMA journal_mode=WAL;", 0, 0, 0);
    const char* sqlUsers = "CREATE TABLE IF NOT EXISTS Users ("
        "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Username TEXT UNIQUE NOT NULL,"
        "PasswordHash TEXT NOT NULL"
        ");";
    if (sqlite3_exec(db,sqlUsers,0, 0, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    const char* sqlMessages = "CREATE TABLE IF NOT EXISTS Messages ("
        "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "SenderUsername TEXT NOT NULL,"
        "ReceiverUsername TEXT NOT NULL,"
        "Content TEXT NOT NULL,"
        "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "IsRead INTEGER DEFAULT 0"
        ");";
    if (sqlite3_exec(db,sqlMessages,0,0, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    sqlite3_close(db);
    return true;
}

bool Database::registerUser(const string& username, const string& password) {
    AutoDBLock lock;
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_open(GetDBPath().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    sqlite3_busy_timeout(db, 5000);

    string hashedPass = Security::hashPassword(password);
    const char* query = "INSERT INTO Users (Username,PasswordHash) VALUES (?,?);";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashedPass.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return ok;
}

bool Database::loginUser(const string& username, const string& password) {
    AutoDBLock lock;
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_open(GetDBPath().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    sqlite3_busy_timeout(db, 5000);

    string hashedPass = Security::hashPassword(password);
    const char* query = "SELECT Id FROM Users WHERE Username = ? AND PasswordHash = ?;";
    if (sqlite3_prepare_v2(db,query, -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashedPass.c_str(), -1, SQLITE_TRANSIENT);
    bool ok = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return ok;
}

bool Database::saveMessage(const string& sender, const string& receiver, const string& content, int isRead) {
    AutoDBLock lock;
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_open(GetDBPath().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    sqlite3_busy_timeout(db, 5000);

    const char* query = "INSERT INTO Messages (SenderUsername,ReceiverUsername,Content,IsRead) VALUES (?,?,?,?);";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    sqlite3_bind_text(stmt,1, sender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,2, receiver.c_str(),-1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,3, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,4, isRead);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return ok;
}

int Database::updateMessagesIsRead(const string& reader, const string& sender) {
    AutoDBLock lock;
    sqlite3* db = nullptr;
    if (sqlite3_open(GetDBPath().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    sqlite3_busy_timeout(db, 5000);

    int changes = 0;
    const char* query = "UPDATE Messages SET IsRead = 1 WHERE ReceiverUsername = ? AND SenderUsername = ? AND IsRead = 0;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, reader.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, sender.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            changes = sqlite3_changes(db);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return changes;
}

vector<HistoryMessage> Database::getChatHistory(const string& user1, const string& user2) {
    AutoDBLock lock;
    vector<HistoryMessage> history;
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(GetDBPath().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return history;
    }
    sqlite3_busy_timeout(db, 5000);
    const char* query = "SELECT SenderUsername, ReceiverUsername, Content, datetime(Timestamp, 'localtime'), IsRead FROM Messages "
        "WHERE (SenderUsername = ? AND ReceiverUsername = ?) "
        "OR (SenderUsername = ? AND ReceiverUsername = ?) "
        "ORDER BY Id ASC;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt,1,user1.c_str(),-1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,2,user2.c_str(),-1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,3,user2.c_str(),-1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,4,user1.c_str(),-1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            HistoryMessage msg{};
            const unsigned char* s = sqlite3_column_text(stmt,0);
            const unsigned char* r = sqlite3_column_text(stmt,1);
            const unsigned char* c = sqlite3_column_text(stmt,2);
            const unsigned char* t = sqlite3_column_text(stmt,3);
            if (s) msg.sender = (const char*)s;
            if (r) msg.receiver = (const char*)r;
            if (c) msg.content = (const char*)c;
            if (t) msg.timestamp = (const char*)t;
            msg.isRead = sqlite3_column_int(stmt, 4);
            history.push_back(msg);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return history;
}

vector<string> Database::getAllUsers() {
    AutoDBLock lock;
    vector<string> users;
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_open(GetDBPath().c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return users;
    }
    sqlite3_busy_timeout(db, 5000);
    const char* query = "SELECT Username FROM Users ORDER BY Username ASC;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* val = sqlite3_column_text(stmt, 0);
            if (val) {
                users.push_back((const char*)val);
            }
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
    return users;
}

