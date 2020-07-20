#include<zmqpp/zmqpp.hpp>
#include<bits/stdc++.h>
#include <thread>         // std::thread
#include"messages/validator.pb.h"
#include"messages/consensus.pb.h"
#include"messages/pbft-message.pb.h"
#ifndef CONSENSUS_HPP
#define CONSENSUS_HPP
namespace iroha{
    namespace consensus{
        class consensusProxy{
            private:
                zmqpp::context ctx;
                std::string engine_endpoint;
                std::shared_ptr<zmqpp::socket> engine_socket;
                std::string network_endpoint;
                std::shared_ptr<zmqpp::socket> network_socket;
                std::string proxy_endpoint;
                std::shared_ptr<zmqpp::socket> proxy_socket;
                
                std::vector<message::ConsensusBlock> blockchain;
                message::ConsensusBlock canditateBlock;
                std::map<std::string, std::string> consensus_settings;
                message::ConsensusPeerInfo localPeer;
                message::ConsensusBlock chainhead;
                std::vector<message::ConsensusPeerInfo> peers;

                struct{
                    std::string name, version;
                    std::vector<message::ConsensusRegisterRequest_Protocol> additional_protocols;
                } EngineInfo;
                std::string getSetting(std::string setting);
                message::Message handleEngineMessage(message::Message request);
                message::ConsensusSettingsGetResponse handleConsensusSettingsGetReq(message::ConsensusSettingsGetRequest request);
                bool sendMsg(std::shared_ptr<zmqpp::socket> socket, std::string id, message::Message responce);
                std::pair<std::string, message::Message> rcvMsg(std::shared_ptr<zmqpp::socket> socket);
                message::ConsensusRegisterResponse handleConsensusRegisterReq(message::ConsensusRegisterRequest request);
                message::ConsensusBroadcastResponse handleConsensusBroadcastReq(message::ConsensusBroadcastRequest request);
                bool registerEngine();
                void handleEngine();
                void testInit();
            public:
                consensusProxy(std::string engine_endpoint, std::string network_endpoint, std::string proxy_endpoint, std::string peerInfo);
                void start();
                void startEngine();
        };
    }
}
#endif