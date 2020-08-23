#include "zmq_channel.hpp"

zmq_channel::zmq_channel() {}
zmq_channel::packet::packet(std::string socket,
                            std::string id,
                            std::string msg) {
  this->socket = socket;
  this->id = id;
  this->msg = msg;
}
void zmq_channel::start() {
  shutdown_mtx.lock();
  ShutdownSignal=false;
  shutdown_mtx.unlock();
  while (true) {
    shutdown_mtx.lock();
    if(ShutdownSignal)
        break;
    shutdown_mtx.unlock();
    sockets_vector_mtx.lock();
    // Pass through the sockets in a Round-Robin Fashion
    for (auto x : sockets) {
      auto the_socket = x.second;
      auto the_id = x.first;
      std::vector<zmq::message_t> input;
      if (recv_multipart(*the_socket,
                         std::back_inserter(input),
                         zmq::recv_flags::dontwait)) {
        std::string peerId =
            std::string(static_cast<char *>(input[0].data()), input[0].size());
        std::string msg =
            std::string(static_cast<char *>(input[1].data()), input[1].size());
        in_queue_mtx.lock();
        input_chan.push(packet(the_id, peerId, msg));
        in_queue_mtx.unlock();
      }
    }
    out_queue_mtx.lock();
    while (!output_chan.empty()) {
      auto temp = output_chan.front();
      output_chan.pop();
      std::vector<zmq::message_t> output;
      if (temp.id.size())
        output.push_back(zmq::message_t(temp.id));
      output.push_back(zmq::message_t(temp.msg));
      zmq::send_multipart(*sockets[temp.socket], output);
    }
    out_queue_mtx.unlock();
    sockets_vector_mtx.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}
void zmq_channel::shutdown(){
    shutdown_mtx.lock();
    ShutdownSignal=true;
    shutdown_mtx.unlock();
}
bool zmq_channel::pop_msg(std::string &socket,
                          std::string &id,
                          std::string &msg) {
  bool flag = false;
  in_queue_mtx.lock();
  if (!input_chan.empty()) {
    auto temp = input_chan.front();
    input_chan.pop();
    id = temp.id;
    socket = temp.socket;
    msg = temp.msg;
    flag = true;
  }
  in_queue_mtx.unlock();
  return flag;
}

bool zmq_channel::push_msg(std::string socket,
                           std::string id,
                           std::string msg) {
  out_queue_mtx.lock();
  // printf("to send to %s\n", socket.c_str());
  output_chan.push(packet(socket, id, msg));
  // printf("%d\n", output_chan.size());
  out_queue_mtx.unlock();
  return true;
}
void zmq_channel::add_socket(std::shared_ptr<zmq::socket_t> newsocket,
                             std::string identity) {
  sockets_vector_mtx.lock();
  sockets.insert({identity, newsocket});
  sockets_vector_mtx.unlock();
  printf("Added Socket %s\n", identity.c_str());
}