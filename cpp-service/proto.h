#pragma once
#include <cstdint>
#include <string>

struct SensorReading {
  int sensor_id = 0;
  double value = 0;
  int64_t timestamp = 0;
  bool ParseFromArray(const void *data, int size);
};

struct ControlCommand {
  uint64_t new_rate = 0;
  std::string target;
  bool ParseFromArray(const void *data, int size);
};
