#include "../generated/data.pb.h"
#include <mutex>
#include <sqlite3.h>
#include <string>

extern "C" {

sqlite3 *service_init(const char *path) {
  sqlite3 *db = nullptr;
  if (sqlite3_open(path, &db) != SQLITE_OK) {
    return nullptr;
  }
  const char *sql = "CREATE TABLE IF NOT EXISTS sensor_readings (sensor_id "
                    "INTEGER, value REAL, timestamp INTEGER);";
  if (sqlite3_exec(db, sql, nullptr, nullptr, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return nullptr;
  }
  return db;
}

void service_close(sqlite3 *db) {
  if (db) {
    sqlite3_close(db);
  }
}

int service_handle_reading(sqlite3 *db, const char *data, int len) {
  if (!db) {
    return 0;
  }
  SensorReading reading;
  if (!reading.ParseFromArray(data, len)) {
    return 0;
  }
  const char *sql =
      "INSERT INTO sensor_readings(sensor_id,value,timestamp) VALUES(?,?,?);";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return 0;
  }
  sqlite3_bind_int(stmt, 1, reading.sensor_id());
  sqlite3_bind_double(stmt, 2, reading.value());
  sqlite3_bind_int64(stmt, 3, reading.timestamp());
  bool ok = sqlite3_step(stmt) == SQLITE_DONE;
  sqlite3_finalize(stmt);
  return ok ? 1 : 0;
}

static std::mutex g_mutex;
static uint64_t g_rate = 0;
static std::string g_target;

void service_process_command(const char *data, int len) {
  ControlCommand cmd;
  if (!cmd.ParseFromArray(data, len)) {
    return;
  }
  std::lock_guard<std::mutex> lock(g_mutex);
  if (cmd.new_rate() != 0) {
    g_rate = cmd.new_rate();
  }
  if (!cmd.target().empty()) {
    g_target = cmd.target();
  }
}

uint64_t service_get_rate() {
  std::lock_guard<std::mutex> lock(g_mutex);
  return g_rate;
}

const char *service_get_target() {
  std::lock_guard<std::mutex> lock(g_mutex);
  return g_target.c_str();
}

} // extern "C"
