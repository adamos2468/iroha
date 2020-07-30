#include<zmqpp/zmqpp.hpp>
#include<bits/stdc++.h>
#include"zmq_channel.hpp"
#include"messages/validator.pb.h"
#include"messages/consensus.pb.h"
#include"messages/pbft-message.pb.h"
#ifndef CONSENSUS_HPP
#define CONSENSUS_HPP
namespace iroha{
    namespace consensus{
        class consensusProxy{
            private:
                bool newblock=false;
                zmq_channel channel;
                std::string myHexId;
                zmqpp::context ctx;
                std::string engine_endpoint;
                std::string engine_id;
                std::shared_ptr<zmqpp::socket> engine_socket;
                std::string network_endpoint;
                std::shared_ptr<zmqpp::socket> network_socket;

                std::string proxy_endpoint;
                std::shared_ptr<zmqpp::socket> proxy_socket;

                std::set<std::string> connected_peers; //ZMQ Stuff
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
                //bool sendMsg(std::shared_ptr<zmqpp::socket> socket, std::string id, message::Message responce);
                std::pair<std::string, message::Message> rcvMsg(std::shared_ptr<zmqpp::socket> socket);
                
                message::ConsensusRegisterResponse          handleConsensusRegisterReq(message::ConsensusRegisterRequest request);
                message::ConsensusBroadcastResponse         handleConsensusBroadcastReq(message::ConsensusBroadcastRequest request);
                message::ConsensusSummarizeBlockResponse    handleBlockSumReq(message::ConsensusSummarizeBlockRequest request);
                message::ConsensusInitializeBlockResponse   handleBlockInitReq(message::ConsensusInitializeBlockRequest request);
                message::ConsensusFinalizeBlockResponse     handleBlockFinalReq(message::ConsensusFinalizeBlockRequest request);
                message::ConsensusCheckBlocksResponse       handleBlockCheckReq(message::ConsensusCheckBlocksRequest request);
                message::ConsensusCommitBlockResponse       handleBlockCommitReq(message::ConsensusCommitBlockRequest request);
                message::ConsensusFailBlockResponse         handleFailBlockReq(message::ConsensusFailBlockRequest request);
                message::ConsensusPeerMessage getPeerMessage(std::string message_type, std::string content);
                void BroadCastCanditateBlock();
                void broadcast(message::ConsensusPeerMessage msg);
                void NotifyBlockNew();
                void NotifyBlockValid(std::string blockid);
                void NotifyBlockInvalid(std::string blockid);
                void NotifyPeerMsg(std::string id, message::ConsensusPeerMessage msg);
                void NotifyBlockCommit(std::string blockid);
                void showPresence();

                bool registerEngine();
                void handleEngine();
                void testInit();
                void networkListener();
                void handlePeerMsg(std::string id, message::ConsensusPeerMessage msg);
                message::ConsensusBlock initializeBlock();
            public:
                consensusProxy(std::string engine_endpoint, std::string network_endpoint, std::string proxy_endpoint, std::string peerInfo);
                void start();
                void startEngine();
        };
    }
}
#endif