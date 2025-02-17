#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "db.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::SetArgReferee;

class SQLiteDBMock : public SQLiteDB {
public:
    SQLiteDBMock() : SQLiteDB("") {}

   
    MOCK_METHOD(bool, execute, (const std::string& sql), (override));

   
    MOCK_METHOD(bool, execute_query, (const std::string& sql, std::vector<std::vector<std::string>>& result), (override));

    
    MOCK_METHOD(sqlite3*, get_db, (), (const, override));
};

// Test cases for EmailLogDB functions
class EmailLogDBTest : public ::testing::Test {
protected:
    SQLiteDBMock dbMock;
};

TEST_F(EmailLogDBTest, TestInsertEmailLog) {
    EXPECT_CALL(dbMock, execute(_))
        .WillOnce(Return(true));

    ASSERT_TRUE(EmailLogDB::insert_email_log(dbMock, "john.doe@example.com", "Test Subject", "Sent"));
}

TEST_F(EmailLogDBTest, TestCountSentEmails) {
    std::vector<std::vector<std::string>> result = {{"5"}};
    EXPECT_CALL(dbMock, execute_query(_, _))
        .WillOnce(Invoke([&](const std::string& sql, std::vector<std::vector<std::string>>& res) {
            res = result;
            return true;
        }));

    
    int count = EmailLogDB::count_sent_emails(dbMock);
    ASSERT_EQ(count, 5);
}

TEST_F(EmailLogDBTest, TestCountEmailsByStatus) {
    std::vector<std::vector<std::string>> result = {{"Sent", "5"}, {"Failed", "3"}};
    EXPECT_CALL(dbMock, execute_query(_, _))
        .WillOnce(Invoke([&](const std::string& sql, std::vector<std::vector<std::string>>& res) {
            res = result;
            return true;
        }));

    auto counts = EmailLogDB::count_emails_by_status(dbMock);
    ASSERT_EQ(counts.size(), 2);
    ASSERT_EQ(counts[0].first, "Sent");
    ASSERT_EQ(counts[0].second, 5);
    ASSERT_EQ(counts[1].first, "Failed");
    ASSERT_EQ(counts[1].second, 3);
}

TEST_F(EmailLogDBTest, TestSuccessRate) {
    std::vector<std::vector<std::string>> totalResult = {{"10"}};
    std::vector<std::vector<std::string>> sentResult = {{"7"}};

    EXPECT_CALL(dbMock, execute_query(_, _))
        .WillOnce(Invoke([&](const std::string& sql, std::vector<std::vector<std::string>>& res) {
            if (sql.find("COUNT(*)") != std::string::npos) {
                res = totalResult;
            } else {
                res = sentResult;
            }
            return true;
        }));

    float rate = EmailLogDB::success_rate(dbMock);
    ASSERT_FLOAT_EQ(rate, 70.0);
}

TEST_F(EmailLogDBTest, TestMostRecentEmailToRecipient) {
    std::vector<std::vector<std::string>> result = {{"Test Subject", "2025-02-18 10:00:00"}};
    EXPECT_CALL(dbMock, execute_query(_, _))
        .WillOnce(Invoke([&](const std::string& sql, std::vector<std::vector<std::string>>& res) {
            res = result;
            return true;
        }));

    // Test fetching the most recent email for a recipient
    auto recentEmail = EmailLogDB::most_recent_email_to_recipient(dbMock, "john.doe@example.com");
    ASSERT_EQ(recentEmail.first, "Test Subject");
    ASSERT_EQ(recentEmail.second, "2025-02-18 10:00:00");
}
