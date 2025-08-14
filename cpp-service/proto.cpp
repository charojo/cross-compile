#include "proto.h"
#include <cstring>

namespace {
bool read_varint(const uint8_t *&data, const uint8_t *end, uint64_t &out) {
  out = 0;
  int shift = 0;
  while (data < end && shift < 64) {
    uint8_t byte = *data++;
    out |= static_cast<uint64_t>(byte & 0x7F) << shift;
    if ((byte & 0x80) == 0) {
      return true;
    }
    shift += 7;
  }
  return false;
}

void skip_field(const uint8_t *&data, const uint8_t *end, uint32_t wire) {
  uint64_t len = 0;
  switch (wire) {
  case 0:
    read_varint(data, end, len);
    break;
  case 1:
    data += 8;
    break;
  case 2:
    if (read_varint(data, end, len)) {
      data += len;
    }
    break;
  case 5:
    data += 4;
    break;
  default:
    data = end;
    break;
  }
}
} // namespace

bool SensorReading::ParseFromArray(const void *data_, int size) {
  const uint8_t *data = static_cast<const uint8_t *>(data_);
  const uint8_t *end = data + size;
  while (data < end) {
    uint64_t key;
    if (!read_varint(data, end, key)) {
      return false;
    }
    uint32_t field = key >> 3;
    uint32_t wire = key & 0x7;
    switch (field) {
    case 1: {
      uint64_t val;
      if (!read_varint(data, end, val)) {
        return false;
      }
      sensor_id = static_cast<int>(val);
      break;
    }
    case 2: {
      if (wire != 1 || data + 8 > end) {
        return false;
      }
      uint64_t bits;
      std::memcpy(&bits, data, 8);
      std::memcpy(&value, &bits, 8);
      data += 8;
      break;
    }
    case 3: {
      uint64_t val;
      if (!read_varint(data, end, val)) {
        return false;
      }
      timestamp = static_cast<int64_t>(val);
      break;
    }
    default:
      skip_field(data, end, wire);
      break;
    }
  }
  return true;
}

bool ControlCommand::ParseFromArray(const void *data_, int size) {
  const uint8_t *data = static_cast<const uint8_t *>(data_);
  const uint8_t *end = data + size;
  while (data < end) {
    uint64_t key;
    if (!read_varint(data, end, key)) {
      return false;
    }
    uint32_t field = key >> 3;
    uint32_t wire = key & 0x7;
    switch (field) {
    case 1: {
      uint64_t val;
      if (!read_varint(data, end, val)) {
        return false;
      }
      new_rate = val;
      break;
    }
    case 2: {
      uint64_t len;
      if (!read_varint(data, end, len) || data + len > end) {
        return false;
      }
      target.assign(reinterpret_cast<const char *>(data), len);
      data += len;
      break;
    }
    default:
      skip_field(data, end, wire);
      break;
    }
  }
  return true;
}
