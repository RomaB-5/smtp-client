#ifndef SQLITEDB_H
#define SQLITEDB_H

#include <sqlite3.h>
#include <string>
#include <vector>

class SQLiteDB {
    public:
        SQLiteDB(const std::string& db_name);
        
        ~SQLiteDB();
    
        bool execute(const std::string& sql);

        bool execute_query(const std::string& sql, std::vector<std::vector<std::string>>& result);
        
        sqlite3* get_db() const;
    
    private:
        sqlite3* db;
    };
    
    namespace EmailLogDB {
    
        bool create_smtp_tables(SQLiteDB& db);
    
        bool insert_email_log(SQLiteDB& db, const std::string& recipient, const std::string& subject, const std::string& status);
    
        int count_sent_emails(SQLiteDB& db);
    
        std::vector<std::pair<std::string, int>> count_emails_by_status(SQLiteDB& db);
    
        std::vector<std::pair<std::string, int>> most_frequent_recipients(SQLiteDB& db);
    
        int average_time_between_emails(SQLiteDB& db);
    
        int longest_time_between_sent_emails(SQLiteDB& db);
    
        float success_rate(SQLiteDB& db);
    
        std::pair<std::string, std::string> most_recent_email_to_recipient(SQLiteDB& db, const std::string& recipient);
    }
    
    #endif // SQLITEDB_H
    