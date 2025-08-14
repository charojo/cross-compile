#include "Database.h"
#include <iostream>

Database::Database(const std::string &dbPath, Cache &cache)
    : db_(nullptr), cache_(cache) {
  if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
    std::cerr << "Cannot open DB: " << sqlite3_errmsg(db_) << std::endl;
  } else {
    const char *sql = "CREATE TABLE IF NOT EXISTS users (user_id INTEGER, "
                      "field TEXT, value TEXT, PRIMARY KEY(user_id, field));";
    char *errMsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
      std::cerr << "Cannot create table: " << errMsg << std::endl;
      sqlite3_free(errMsg);
    }
  }
}

Database::~Database() {
  if (db_) {
    sqlite3_close(db_);
  }
}

bool Database::UpdateUser(int userId, const std::string &field,
                          const std::string &value) {
  const char *sql =
      "INSERT INTO users (user_id, field, value) VALUES (?,?,?) ON "
      "CONFLICT(user_id, field) DO UPDATE SET value=excluded.value;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }
  sqlite3_bind_int(stmt, 1, userId);
  sqlite3_bind_text(stmt, 2, field.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, value.c_str(), -1, SQLITE_TRANSIENT);
  bool ok = sqlite3_step(stmt) == SQLITE_DONE;
  sqlite3_finalize(stmt);
  if (ok) {
    cache_.Set(userId, field, value);
  }
  return ok;
}

std::string Database::GetUser(int userId, const std::string &field) {
  if (field.empty()) {
    return "";
  }
  std::string cached = cache_.Get(userId, field);
  if (!cached.empty()) {
    return cached;
  }
  const char *sql = "SELECT value FROM users WHERE user_id=? AND field=?;";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return "";
  }
  sqlite3_bind_int(stmt, 1, userId);
  sqlite3_bind_text(stmt, 2, field.c_str(), -1, SQLITE_TRANSIENT);
  std::string value;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char *text = sqlite3_column_text(stmt, 0);
    if (text) {
      value = reinterpret_cast<const char *>(text);
    }
  }
  sqlite3_finalize(stmt);
  if (!value.empty()) {
    cache_.Set(userId, field, value);
  }
  return value;
}
