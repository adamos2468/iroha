/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <bits/stdc++.h>
#include <atomic>
#include <condition_variable>
#include <thread>

#include <rxcpp/subjects/rx-subject.hpp>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "consensus/round.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "messages/consensus.pb.h"
#include "messages/pbft-message.pb.h"
#include "messages/validator.pb.h"
#include "zmq_channel.hpp"

#ifndef IROHA_CONSENSUS_PROXY_CONSENSUS_HPP
#define IROHA_CONSENSUS_PROXY_CONSENSUS_HPP

namespace iroha {
  namespace ametsuchi {
    class SettingQuery;
  }

  namespace synchronizer {
    class Synchronizer;
  }

  namespace consensus {
    class consensusProxy {
     private:
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

      class PromisedBlock {
       public:
        PromisedBlock(std::future<BlockCreationResult> future_block);

        BlockCreationResult &get();

       private:
        std::future<BlockCreationResult> promised_block_;
        std::optional<BlockCreationResult> block_;
      };
      std::optional<PromisedBlock> candidate_block_;

      std::shared_ptr<iroha::ametsuchi::SettingQuery> settings_query_;
      std::shared_ptr<iroha::simulator::Simulator> simulator_;

      // std::atomic_bool stop_;

      std::shared_ptr<iroha::synchronizer::Synchronizer> synchronizer_;

      /*
      std::thread synchronizer_thread_;
      std::mutex synchronization_mutex_;
      bool synchronization_requested_;
      std::condition_variable synchronization_cv_;
      */
      /*
      rxcpp::observe_on_one_worker synchronizer_worker_;
      rxcpp::composite_subscription synchronizer_subscription_;
      rxcpp::subjects::subject<void> synchronizer_subject_;
      */

      struct {
        std::string name, version;
        std::vector<message::ConsensusRegisterRequest_Protocol>
            additional_protocols;
      } EngineInfo;
      std::optional<std::string> getSetting(std::string setting);
      message::Message handleEngineMessage(message::Message request);
      message::ConsensusSettingsGetResponse handleConsensusSettingsGetReq(
          message::ConsensusSettingsGetRequest request);

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
      PromisedBlock consensusProxy::prepareBlockAsync(
          std::shared_ptr<const shared_model::interface::Proposal> proposal);
      void synchronizeAsync() const;
      std::string getBlockId(shared_model::interface::Block const &) const;
      void updateBlockId(shared_model::interface::Block &) const;
      BlockCreationResult &getCandidateBlock();

     public:
      consensusProxy(
          std::string engine_endpoint,
          std::string network_endpoint,
          std::string proxy_endpoint,
          std::string peerInfo,
          std::shared_ptr<iroha::ametsuchi::SettingQuery> settings_query);
      void start();
      void startEngine();
    };
  }  // namespace consensus
}  // namespace iroha
#endif
