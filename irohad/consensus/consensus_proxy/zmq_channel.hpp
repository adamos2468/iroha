#include<zmq.hpp>
#include <zmq_addon.hpp>
#include<bits/stdc++.h>
#include <thread>         // std::thread
#include "messages/consensus.pb.h"
#include "messages/validator.pb.h"
#ifndef ZMQ_CHANNEL_HPP
#define ZMQ_CHANNEL_HPP
class zmq_channel{
    private:
        struct packet{
            std::string socket;
            std::string id;
            std::string msg;
            packet(std::string socket, std::string id, std::string msg);
        }; 
        std::mutex sockets_vector_mtx;
        std::mutex in_queue_mtx;
        std::mutex out_queue_mtx;
        std::map< std::string, std::shared_ptr<zmq::socket_t> > sockets;
        std::queue<packet> input_chan;
        std::queue<packet> output_chan;
    public:
        bool pop_msg(std::string &socket, std::string &id, std::string &msg);
        bool push_msg(std::string socket, std::string id, std::string msg);
        zmq_channel();
        void add_socket(std::shared_ptr<zmq::socket_t> newsocket, std::string identity);
        void start();
};
#endif