#pragma once

#include "Cache.h"
#include <sqlite3.h>
#include <string>

class Database {
public:
  Database(const std::string &dbPath, Cache &cache);
  ~Database();

  bool UpdateUser(int userId, const std::string &field,
                  const std::string &value);
  std::string GetUser(int userId, const std::string &field);

private:
  sqlite3 *db_;
  Cache &cache_;
};
