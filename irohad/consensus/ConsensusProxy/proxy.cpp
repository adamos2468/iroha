#include<bits/stdc++.h>
#include<zmqpp/zmqpp.hpp>
#include<zmqpp/proxy.hpp>
//Just Testing
int main(){
    zmqpp::context ctx;
    zmqpp::socket capture=zmqpp::socket(ctx, zmqpp::socket_type::router);
    zmqpp::socket sender=zmqpp::socket(ctx, zmqpp::socket_type::pub);

    capture.bind("tcp://*:7777");
    sender.bind("tcp://*:7778");

    zmqpp::proxy(capture, sender);
}