#include"zmq_channel.hpp"

zmq_channel::zmq_channel(){
}
zmq_channel::packet::packet(std::string socket, std::string id, std::string msg){
    this->socket=socket;
    this->id=id;
    this->msg=msg;
}
void zmq_channel::start(){
    while(true){
        sockets_vector_mtx.lock();
        //Pass through the sockets in a Round-Robin Fashion
        for(auto x:sockets){
            auto the_socket=x.second;
            auto the_id=x.first;
            zmqpp::message input;
            if(the_socket->receive(input, true)){
                std::string peerId, msg;
                input>>peerId>>msg;
                in_queue_mtx.lock();
                input_chan.push(packet(the_id, peerId, msg));
                in_queue_mtx.unlock();
            }
        }
        out_queue_mtx.lock();
        while(!output_chan.empty()){
            auto temp=output_chan.front();
            output_chan.pop();
            zmqpp::message output;
            if(temp.id.size())
                output<<temp.id;
            output<<temp.msg;
            if(temp.socket=="engine"){
                iroha::consensus::message::Message msg;
                msg.ParseFromString(temp.msg);
            }
            sockets[temp.socket]->send(output);
        }
        out_queue_mtx.unlock();
        sockets_vector_mtx.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

bool zmq_channel::pop_msg(std::string &socket, std::string &id, std::string &msg){
    bool flag=false;
    in_queue_mtx.lock();
    if(!input_chan.empty()){
        auto temp=input_chan.front();
        input_chan.pop();
        id=temp.id;
        socket=temp.socket;
        msg=temp.msg;
        flag=true;
    }
    in_queue_mtx.unlock();
    return flag;
}

bool zmq_channel::push_msg(std::string socket, std::string id, std::string msg){
    out_queue_mtx.lock();
    //printf("to send to %s\n", socket.c_str());
    output_chan.push(packet(socket, id, msg));
    //printf("%d\n", output_chan.size());
    out_queue_mtx.unlock();
    return true;
}

void zmq_channel::add_socket(std::shared_ptr<zmqpp::socket> newsocket, std::string identity){
    sockets_vector_mtx.lock();
    sockets.insert({identity, newsocket});
    sockets_vector_mtx.unlock();
    printf("Added Socket %s\n", identity.c_str());
}