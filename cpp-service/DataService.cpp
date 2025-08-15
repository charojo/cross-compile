#include "Cache.h"
#include "Database.h"
#include "generated/data.pb.h"

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <zmq.h>

namespace {
std::atomic<bool> running{true};

void handle_signal(int signum) {
  std::cout << "Signal " << signum << " received, initiating shutdown"
            << std::endl;
  running = false;
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
        if (req.ParseFromString(payload)) {
          std::string old_value = db.GetUser(req.user_id(), req.field());
          bool ok = db.UpdateUser(req.user_id(), req.field(), req.value());
          UpdateUserResponse resp;
          resp.set_success(ok);
          resp.set_message(ok ? "ok" : "error");
          std::string resp_data;
          resp.SerializeToString(&resp_data);
          const char *resp_type = "UpdateUserResponse";
          zmq_send(rep, resp_type, std::strlen(resp_type), ZMQ_SNDMORE);
          zmq_send(rep, resp_data.data(), resp_data.size(), 0);
          if (ok) {
            UpdateUserEvent event;
            event.set_user_id(req.user_id());
            event.set_field(req.field());
            if (!old_value.empty()) {
              event.set_old_value(old_value);
            }
            event.set_new_value(req.value());
            std::string event_data;
            event.SerializeToString(&event_data);
            const char *event_type = "UpdateUserEvent";
            zmq_send(pub, event_type, std::strlen(event_type), ZMQ_SNDMORE);
            zmq_send(pub, event_data.data(), event_data.size(), 0);
            std::cout << "Updated user " << req.user_id() << " field "
                      << req.field() << std::endl;
          }
        } else {
          std::cerr << "failed to parse UpdateUserRequest" << std::endl;
          UpdateUserResponse resp;
          resp.set_success(false);
          resp.set_message("invalid");
          std::string resp_data;
          resp.SerializeToString(&resp_data);
          const char *resp_type = "UpdateUserResponse";
          zmq_send(rep, resp_type, std::strlen(resp_type), ZMQ_SNDMORE);
          zmq_send(rep, resp_data.data(), resp_data.size(), 0);
        }
      } else if (mtype == "GetUserRequest") {
        GetUserRequest req;
        if (req.ParseFromString(payload)) {
          std::string value = db.GetUser(req.user_id(), req.field());
          bool found = !value.empty();
          GetUserResponse resp;
          resp.set_found(found);
          if (found) {
            resp.set_value(value);
          }
          std::string resp_data;
          resp.SerializeToString(&resp_data);
          const char *resp_type = "GetUserResponse";
          zmq_send(rep, resp_type, std::strlen(resp_type), ZMQ_SNDMORE);
          zmq_send(rep, resp_data.data(), resp_data.size(), 0);
          std::cout << "Get user " << req.user_id() << " field " << req.field()
                    << std::endl;
        } else {
          std::cerr << "failed to parse GetUserRequest" << std::endl;
          GetUserResponse resp;
          resp.set_found(false);
          std::string resp_data;
          resp.SerializeToString(&resp_data);
          const char *resp_type = "GetUserResponse";
          zmq_send(rep, resp_type, std::strlen(resp_type), ZMQ_SNDMORE);
          zmq_send(rep, resp_data.data(), resp_data.size(), 0);
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
