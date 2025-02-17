#include "SQLiteDB.h"
#include <iostream>
#include <sqlite3.h>
#include <cstdlib>
#include <utility>
#include <sstream>

// SQLiteDB class implementation

SQLiteDB::SQLiteDB(const std::string& db_name) {
    int rc = sqlite3_open(db_name.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        std::exit(1); // Exiting due to failed DB connection
    } else {
        std::cout << "Opened database successfully\n";
    }
}

SQLiteDB::~SQLiteDB() {
    sqlite3_close(db);
}

bool SQLiteDB::execute(const std::string& sql) {
    char* errMessage = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMessage << std::endl;
        sqlite3_free(errMessage);
        return false;
    }
    return true;
}

bool SQLiteDB::execute_query(const std::string& sql, std::vector<std::vector<std::string>>& result) {
    char* errMessage = nullptr;
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to fetch data: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    int colCount = sqlite3_column_count(stmt);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::vector<std::string> row;
        for (int i = 0; i < colCount; i++) {
            row.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
        }
        result.push_back(row);
    }

    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

sqlite3* SQLiteDB::get_db() const {
    return db;
}

// EmailLogDB namespace implementation

bool EmailLogDB::create_smtp_tables(SQLiteDB& db) {
    std::string create_email_log_table = 
    "CREATE TABLE IF NOT EXISTS email_log ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "recipient TEXT, "
    "subject TEXT, "
    "status TEXT, "
    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    return db.execute(create_email_log_table);
}

bool EmailLogDB::insert_email_log(SQLiteDB& db, const std::string& recipient, const std::string& subject, const std::string& status) {
    std::string insert_sql = "INSERT INTO email_log (recipient, subject, status) VALUES ('" 
                            + recipient + "', '" 
                            + subject + "', '" 
                            + status + "');";
    return db.execute(insert_sql);
}

int EmailLogDB::count_sent_emails(SQLiteDB& db) {
    std::string count_sql = "SELECT COUNT(*) FROM email_log WHERE status = 'Sent';";
    std::vector<std::vector<std::string>> result;

    if (db.execute_query(count_sql, result) && !result.empty()) {
        return std::stoi(result[0][0]); // First column of the first row contains the count
    }
    return 0;
}

std::vector<std::pair<std::string, int>> EmailLogDB::count_emails_by_status(SQLiteDB& db) {
    std::string count_sql = "SELECT status, COUNT(*) FROM email_log GROUP BY status;";
    std::vector<std::vector<std::string>> result;
    std::vector<std::pair<std::string, int>> counts;

    if (db.execute_query(count_sql, result)) {
        for (const auto& row : result) {
            counts.push_back({row[0], std::stoi(row[1])});
        }
    }
    return counts;
}

std::vector<std::pair<std::string, int>> EmailLogDB::most_frequent_recipients(SQLiteDB& db) {
    std::string sql = "SELECT recipient, COUNT(*) as count FROM email_log GROUP BY recipient ORDER BY count DESC LIMIT 5;";
    std::vector<std::vector<std::string>> result;
    std::vector<std::pair<std::string, int>> frequent_recipients;

    if (db.execute_query(sql, result)) {
        for (const auto& row : result) {
            frequent_recipients.push_back({row[0], std::stoi(row[1])});
        }
    }
    return frequent_recipients;
}

int EmailLogDB::average_time_between_emails(SQLiteDB& db) {
    std::string sql = 
        "SELECT AVG(strftime('%s', timestamp) - strftime('%s', LAG(timestamp) OVER (ORDER BY timestamp))) "
        "FROM email_log WHERE status = 'Sent';";
    
    std::vector<std::vector<std::string>> result;

    if (db.execute_query(sql, result) && !result.empty()) {
        return std::stoi(result[0][0]); // Returns time in seconds
    }
    return 0;
}

int EmailLogDB::longest_time_between_sent_emails(SQLiteDB& db) {
    std::string sql = 
        "SELECT MAX(strftime('%s', timestamp) - strftime('%s', LAG(timestamp) OVER (ORDER BY timestamp))) "
        "FROM email_log WHERE status = 'Sent';";
    
    std::vector<std::vector<std::string>> result;

    if (db.execute_query(sql, result) && !result.empty()) {
        return std::stoi(result[0][0]); // Returns time in seconds
    }
    return 0;
}

float EmailLogDB::success_rate(SQLiteDB& db) {
    std::string total_sql = "SELECT COUNT(*) FROM email_log;";
    std::string sent_sql = "SELECT COUNT(*) FROM email_log WHERE status = 'Sent';";

    std::vector<std::vector<std::string>> total_result;
    std::vector<std::vector<std::string>> sent_result;

    if (db.execute_query(total_sql, total_result) && !total_result.empty() &&
        db.execute_query(sent_sql, sent_result) && !sent_result.empty()) {
        
        int total_emails = std::stoi(total_result[0][0]);
        int sent_emails = std::stoi(sent_result[0][0]);
        
        if (total_emails > 0) {
            return static_cast<float>(sent_emails) / total_emails * 100;
        }
    }
    return 0.0f;
}

std::pair<std::string, std::string> EmailLogDB::most_recent_email_to_recipient(SQLiteDB& db, const std::string& recipient) {
    std::string sql = 
        "SELECT subject, timestamp FROM email_log WHERE recipient = ? ORDER BY timestamp DESC LIMIT 1;";
    
    sqlite3_stmt* stmt;
    std::pair<std::string, std::string> result = {"", ""};

    if (sqlite3_prepare_v2(db.get_db(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare query: " << sqlite3_errmsg(db.get_db()) << std::endl;
        return result;
    }
    sqlite3_bind_text(stmt, 1, recipient.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result.first = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        result.second = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }

    sqlite3_finalize(stmt);
    return result;
}
