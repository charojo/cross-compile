#include "Cache.h"
#include "Database.h"

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <string>
#include <zmq.h>

namespace {
std::atomic<bool> running{true};

void handle_signal(int) { running = false; }

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

bool parse_string(const uint8_t *&data, const uint8_t *end, std::string &out) {
  uint64_t len;
  if (!read_varint(data, end, len) || data + len > end) {
    return false;
  }
  out.assign(reinterpret_cast<const char *>(data), len);
  data += len;
  return true;
}

struct UpdateUserRequest {
  int user_id = 0;
  std::string field;
  std::string value;
  bool Parse(const void *buf, int size) {
    const uint8_t *data = static_cast<const uint8_t *>(buf);
    const uint8_t *end = data + size;
    while (data < end) {
      uint64_t key;
      if (!read_varint(data, end, key)) {
        return false;
      }
      uint32_t fieldNum = key >> 3;
      uint32_t wire = key & 0x7;
      switch (fieldNum) {
      case 1: {
        uint64_t v;
        if (!read_varint(data, end, v)) {
          return false;
        }
        user_id = static_cast<int>(v);
        break;
      }
      case 2:
        if (!parse_string(data, end, field)) {
          return false;
        }
        break;
      case 3:
        if (!parse_string(data, end, value)) {
          return false;
        }
        break;
      default:
        skip_field(data, end, wire);
        break;
      }
    }
    return true;
  }
};

struct GetUserRequest {
  int user_id = 0;
  std::string field;
  bool Parse(const void *buf, int size) {
    const uint8_t *data = static_cast<const uint8_t *>(buf);
    const uint8_t *end = data + size;
    while (data < end) {
      uint64_t key;
      if (!read_varint(data, end, key)) {
        return false;
      }
      uint32_t fieldNum = key >> 3;
      uint32_t wire = key & 0x7;
      switch (fieldNum) {
      case 1: {
        uint64_t v;
        if (!read_varint(data, end, v)) {
          return false;
        }
        user_id = static_cast<int>(v);
        break;
      }
      case 2:
        if (!parse_string(data, end, field)) {
          return false;
        }
        break;
      default:
        skip_field(data, end, wire);
        break;
      }
    }
    return true;
  }
};

void write_varint(std::string &out, uint64_t value) {
  while (value > 0x7F) {
    out.push_back(static_cast<char>((value & 0x7F) | 0x80));
    value >>= 7;
  }
  out.push_back(static_cast<char>(value & 0x7F));
}

void write_key(std::string &out, uint32_t field, uint32_t wire) {
  write_varint(out, (static_cast<uint64_t>(field) << 3) | wire);
}

void write_int(std::string &out, uint32_t field, uint64_t value) {
  write_key(out, field, 0);
  write_varint(out, value);
}

void write_bool(std::string &out, uint32_t field, bool value) {
  write_int(out, field, value ? 1 : 0);
}

void write_string(std::string &out, uint32_t field, const std::string &value) {
  write_key(out, field, 2);
  write_varint(out, value.size());
  out.append(value);
}

std::string make_update_user_response(bool success,
                                      const std::string &message) {
  std::string out;
  write_bool(out, 1, success);
  write_string(out, 2, message);
  return out;
}

std::string make_get_user_response(bool found, const std::string &value) {
  std::string out;
  write_bool(out, 1, found);
  if (found) {
    write_string(out, 2, value);
  }
  return out;
}

std::string make_update_user_event(int user_id, const std::string &field,
                                   const std::string &old_value,
                                   const std::string &new_value) {
  std::string out;
  write_int(out, 1, static_cast<uint64_t>(user_id));
  write_string(out, 2, field);
  if (!old_value.empty()) {
    write_string(out, 3, old_value);
  }
  write_string(out, 4, new_value);
  write_string(out, 5, new_value);
  return out;
}
} // namespace

int main() {
  std::signal(SIGINT, handle_signal);
  std::signal(SIGTERM, handle_signal);

  std::cout << "Data Service running" << std::endl;

  Cache cache;
  Database db("users.db", cache);

  void *ctx = zmq_ctx_new();
  void *rep = zmq_socket(ctx, ZMQ_REP);
  if (zmq_bind(rep, "tcp://127.0.0.1:5555") != 0) {
    std::cerr << "Failed to bind REP socket: " << zmq_strerror(zmq_errno())
              << std::endl;
    zmq_close(rep);
    zmq_ctx_term(ctx);
    return 1;
  }
  void *pub = zmq_socket(ctx, ZMQ_PUB);
  if (zmq_bind(pub, "tcp://127.0.0.1:5556") != 0) {
    std::cerr << "Failed to bind PUB socket: " << zmq_strerror(zmq_errno())
              << std::endl;
    zmq_close(rep);
    zmq_close(pub);
    zmq_ctx_term(ctx);
    return 1;
  }

  zmq_pollitem_t items[] = {{rep, 0, ZMQ_POLLIN, 0}};
  while (running) {
    int rc = zmq_poll(items, 1, 100);
    if (rc == -1) {
      if (zmq_errno() == EINTR) {
        continue;
      }
      std::cerr << "zmq_poll failed: " << zmq_strerror(zmq_errno())
                << std::endl;
      break;
    }
    if (items[0].revents & ZMQ_POLLIN) {
      zmq_msg_t type_msg;
      zmq_msg_t payload_msg;
      zmq_msg_init(&type_msg);
      zmq_msg_init(&payload_msg);
      if (zmq_msg_recv(&type_msg, rep, 0) == -1) {
        std::cerr << "failed to receive type frame" << std::endl;
        zmq_msg_close(&type_msg);
        zmq_msg_close(&payload_msg);
        continue;
      }
      if (zmq_msg_recv(&payload_msg, rep, 0) == -1) {
        std::cerr << "failed to receive payload frame" << std::endl;
        zmq_msg_close(&type_msg);
        zmq_msg_close(&payload_msg);
        continue;
      }
      std::string mtype(static_cast<char *>(zmq_msg_data(&type_msg)),
                        zmq_msg_size(&type_msg));
      std::string payload(static_cast<char *>(zmq_msg_data(&payload_msg)),
                          zmq_msg_size(&payload_msg));
      zmq_msg_close(&type_msg);
      zmq_msg_close(&payload_msg);

      std::cout << "Received " << mtype << std::endl;
      if (mtype == "UpdateUserRequest") {
        UpdateUserRequest req;
        if (req.Parse(payload.data(), static_cast<int>(payload.size()))) {
          std::string old_value = db.GetUser(req.user_id, req.field);
          bool ok = db.UpdateUser(req.user_id, req.field, req.value);
          auto resp = make_update_user_response(ok, ok ? "ok" : "error");
          zmq_send(rep, "UpdateUserResponse", 17, ZMQ_SNDMORE);
          zmq_send(rep, resp.data(), resp.size(), 0);
          if (ok) {
            auto event = make_update_user_event(req.user_id, req.field,
                                                old_value, req.value);
            zmq_send(pub, "UpdateUserEvent", 15, ZMQ_SNDMORE);
            zmq_send(pub, event.data(), event.size(), 0);
            std::cout << "Updated user " << req.user_id << " field "
                      << req.field << std::endl;
          }
        } else {
          std::cerr << "failed to parse UpdateUserRequest" << std::endl;
          auto resp = make_update_user_response(false, "invalid");
          zmq_send(rep, "UpdateUserResponse", 17, ZMQ_SNDMORE);
          zmq_send(rep, resp.data(), resp.size(), 0);
        }
      } else if (mtype == "GetUserRequest") {
        GetUserRequest req;
        if (req.Parse(payload.data(), static_cast<int>(payload.size()))) {
          std::string value = db.GetUser(req.user_id, req.field);
          bool found = !value.empty();
          auto resp = make_get_user_response(found, value);
          zmq_send(rep, "GetUserResponse", 15, ZMQ_SNDMORE);
          zmq_send(rep, resp.data(), resp.size(), 0);
          std::cout << "Get user " << req.user_id << " field " << req.field
                    << std::endl;
        } else {
          std::cerr << "failed to parse GetUserRequest" << std::endl;
          auto resp = make_get_user_response(false, "");
          zmq_send(rep, "GetUserResponse", 15, ZMQ_SNDMORE);
          zmq_send(rep, resp.data(), resp.size(), 0);
        }
      } else {
        std::cerr << "unknown message type: " << mtype << std::endl;
        zmq_send(rep, "", 0, 0);
      }
    }
  }

  zmq_close(rep);
  zmq_close(pub);
  zmq_ctx_term(ctx);
  std::cout << "Data Service shutting down" << std::endl;
  return 0;
}
