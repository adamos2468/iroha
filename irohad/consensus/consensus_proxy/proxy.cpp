#include <bits/stdc++.h>
#include <zmqpp/proxy.hpp>
#include <zmqpp/zmqpp.hpp>
#include "messages/consensus.pb.h"
// Just Testing
std::string hexToBin(std::string hex) {
  std::string ans;
  for (int i = 0; i < hex.size(); i += 2) {
    int temp = 0;
    if ('A' <= hex[i] && hex[i] <= 'F')
      temp = hex[i] - 'A' + 10;
    else
      temp = hex[i] - '0';
    temp <<= 4;
    if ('A' <= hex[i + 1] && hex[i + 1] <= 'F')
      temp += hex[i + 1] - 'A' + 10;
    else
      temp += hex[i + 1] - '0';
    ans.push_back(temp);
  }
  return ans;
}
std::string binToHex(std::string bin) {
  std::string hex;
  for (int i = 0; i < bin.size(); i++) {
    std::string temp;
    int last = bin[i] & ((1 << 4) - 1);
    if (last >= 10)
      temp += 'A' + last - 10;
    else
      temp += '0' + last;
    last = (unsigned char)bin[i] >> 4;
    if (last >= 10)
      temp = (char)('A' + last - 10) + temp;
    else
      temp = (char)('0' + last) + temp;
    hex += temp;
  }
  return hex;
}
void copy(zmqpp::socket *printer) {
  while (true) {
    zmqpp::message input;
    printer->receive(input);
    std::string id, msg;
    input >> id >> msg;
    iroha::consensus::message::ConsensusPeerMessage decmsg;
    iroha::consensus::message::ConsensusPeerMessageHeader header;
    decmsg.ParseFromString(msg);
    header.ParseFromString(decmsg.header());
    std::cerr << "Received " << header.message_type() << " from "
              << binToHex(id) << std::endl;
  }
}
int main() {
  zmqpp::context ctx;
  zmqpp::socket capture = zmqpp::socket(ctx, zmqpp::socket_type::router);
  zmqpp::socket sender = zmqpp::socket(ctx, zmqpp::socket_type::pub);
  zmqpp::socket logger = zmqpp::socket(ctx, zmqpp::socket_type::pair);
  zmqpp::socket printer = zmqpp::socket(ctx, zmqpp::socket_type::pair);

  logger.bind("inproc://stalker");
  printer.connect("inproc://stalker");
  capture.bind("tcp://*:7777");
  sender.bind("tcp://*:7778");
  std::thread logging(copy, &printer);

  zmqpp::proxy(capture, sender, logger);
  logging.join();
}