#include <optional>

#include <bits/stdc++.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "interfaces/iroha_internal/block.hpp"
#include "messages/consensus.pb.h"
#include "messages/pbft-message.pb.h"
#include "messages/validator.pb.h"
#include "zmq_channel.hpp"
#ifndef CONSENSUS_HPP
#define CONSENSUS_HPP
namespace iroha {
  namespace consensus {
    class consensusProxy {
     private:
      bool newblock = false;
      zmq_channel channel;
      std::string myHexId;
      zmq::context_t ctx;
      std::string engine_endpoint;
      std::string engine_id;
      std::shared_ptr<zmq::socket_t> engine_socket;
      std::string network_endpoint;
      std::shared_ptr<zmq::socket_t> network_socket;

      std::string proxy_endpoint;
      std::shared_ptr<zmq::socket_t> proxy_socket;

      std::set<std::string> connected_peers;  // ZMQ Stuff
      std::vector<message::ConsensusBlock> blockchain;
      message::ConsensusBlock canditateBlock;
      std::map<std::string, std::string> consensus_settings;
      message::ConsensusPeerInfo localPeer;
      message::ConsensusBlock chainhead;
      std::vector<message::ConsensusPeerInfo> peers;

      using BlockCreationResult =
          std::optional<std::shared_ptr<shared_model::interface::Block>>;
      BlockCreationResult candidate_block_;

      struct {
        std::string name, version;
        std::vector<message::ConsensusRegisterRequest_Protocol>
            additional_protocols;
      } EngineInfo;
      std::string getSetting(std::string setting);
      message::Message handleEngineMessage(message::Message request);
      message::ConsensusSettingsGetResponse handleConsensusSettingsGetReq(
          message::ConsensusSettingsGetRequest request);

      std::optional<std::shared_ptr<shared_model::interface::Block const>>
          condidate_block_;

      message::ConsensusRegisterResponse handleConsensusRegisterReq(
          message::ConsensusRegisterRequest request);
      message::ConsensusBroadcastResponse handleConsensusBroadcastReq(
          message::ConsensusBroadcastRequest request);
      message::ConsensusSummarizeBlockResponse handleBlockSumReq(
          message::ConsensusSummarizeBlockRequest request);
      message::ConsensusInitializeBlockResponse handleBlockInitReq(
          message::ConsensusInitializeBlockRequest request);
      message::ConsensusFinalizeBlockResponse handleBlockFinalReq(
          message::ConsensusFinalizeBlockRequest request);
      message::ConsensusCheckBlocksResponse handleBlockCheckReq(
          message::ConsensusCheckBlocksRequest request);
      message::ConsensusCommitBlockResponse handleBlockCommitReq(
          message::ConsensusCommitBlockRequest request);
      message::ConsensusFailBlockResponse handleFailBlockReq(
          message::ConsensusFailBlockRequest request);
      message::ConsensusPeerMessage getPeerMessage(std::string message_type,
                                                   std::string content);
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
      BlockCreationResult &getCandidateBlock();

     public:
      consensusProxy(std::string engine_endpoint,
                     std::string network_endpoint,
                     std::string proxy_endpoint,
                     std::string peerInfo);
      void start();
      void startEngine();
    };
  }  // namespace consensus
}  // namespace iroha
#endif
