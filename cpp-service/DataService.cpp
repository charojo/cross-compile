
#include "Cache.h"
#include "Database.h"
#include "data.pb.h"
#include <iostream>
#include <string>
#include <zmq.hpp>

int main() {
  std::cout << "Data Service running" << std::endl;
  Cache cache;
  Database db("users.db", cache);

  zmq::context_t context{1};
  zmq::socket_t rep{context, zmq::socket_type::rep};
  rep.bind("tcp://0.0.0.0:5555");
  zmq::socket_t pub{context, zmq::socket_type::pub};
  pub.bind("tcp://0.0.0.0:5556");

  while (true) {
    zmq::message_t typeMsg;
    zmq::message_t dataMsg;
    if (!rep.recv(typeMsg, zmq::recv_flags::none)) {
      break;
    }
    if (!rep.recv(dataMsg, zmq::recv_flags::none)) {
      break;
    }
    std::string type(static_cast<char *>(typeMsg.data()), typeMsg.size());

    if (type == "UpdateUserRequest") {
      UpdateUserRequest req;
      req.ParseFromArray(dataMsg.data(), dataMsg.size());
      bool success = db.UpdateUser(req.user_id(), req.field(), req.value());
      UpdateUserResponse resp;
      resp.set_success(success);
      resp.set_message(success ? "updated" : "error");
      std::string respStr;
      resp.SerializeToString(&respStr);
      rep.send(zmq::str_buffer("UpdateUserResponse"), zmq::send_flags::sndmore);
      rep.send(zmq::buffer(respStr));

      UpdateUserEvent ev;
      ev.set_user_id(req.user_id());
      ev.set_field(req.field());
      ev.set_value(req.value());
      std::string evStr;
      ev.SerializeToString(&evStr);
      pub.send(zmq::str_buffer("UpdateUserEvent"), zmq::send_flags::sndmore);
      pub.send(zmq::buffer(evStr));
    } else if (type == "GetUserRequest") {
      GetUserRequest req;
      req.ParseFromArray(dataMsg.data(), dataMsg.size());
      GetUserResponse resp;
      std::string value = db.GetUser(req.user_id(), req.field());
      if (!value.empty()) {
        resp.set_found(true);
        resp.set_value(value);
      } else {
        resp.set_found(false);
      }
      std::string respStr;
      resp.SerializeToString(&respStr);
      rep.send(zmq::str_buffer("GetUserResponse"), zmq::send_flags::sndmore);
      rep.send(zmq::buffer(respStr));
    } else {
      UpdateUserResponse resp;
      resp.set_success(false);
      resp.set_message("unknown request");
      std::string respStr;
      resp.SerializeToString(&respStr);
      rep.send(zmq::str_buffer("UpdateUserResponse"), zmq::send_flags::sndmore);
      rep.send(zmq::buffer(respStr));
    }
  }

  return 0;
}
