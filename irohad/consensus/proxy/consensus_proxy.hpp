/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_CONSENSUS_PROXY_HPP
#define IROHA_CONSENSUS_PROXY_HPP

#include <future>
#include <optional>

#include "consensus/round.hpp"
#include "interfaces/common_objects/transaction_sequence_common.hpp"
#include "simulator/verified_proposal_creator_common.hpp"

namespace iroha {
  namespace ordering::transport {
    class OdOsNotification;
  }
  namespace simulator {
    class Simulator;
  }
}  // namespace iroha

namespace iroha::consensus::proxy {
  class Proxy {
   public:
   private:
    ConsensusInitializeBlockResponse handleInitializeBlock(
        ConsensusInitializeBlockRequest const &request);

    ConsensusSummarizeBlockResponse handleSummarizeBlock(
        ConsensusSummarizeBlockRequest const &request);

    ConsensusFinalizeBlockResponse handleFinalizeBlock(
        ConsensusFinalizeBlockRequest const &request);

    ConsensusCancelBlockResponse handleConsensusCancelBlock(
        ConsensusCancelBlockRequest const &request);

    ConsensusCheckBlocksResponse handleCheckBlocks(
        ConsensusCheckBlocksRequest const &request);

    std::unique_ptr<iroha::ordering::transport::OdOsNotification>
        ordering_network_client_;
    iroha::consensus::Round ordering_round_;

    std::shared_ptr<iroha::simulator::Simulator> simulator_;

    class PromisedBlock {
     public:
      using BlockCreationResult =
          std::optional<std::shared_ptr<shared_model::interface::Block>>;

      PromisedBlock(std::future<BlockCreationResult> future_block);

      BlockCreationResult &get();

     private:
      std::future<BlockCreationResult> candidate_block_;
      std::optional<BlockCreationResult> block_;
    };
    std::optional<PromisedBlock> promised_block_;
  };
}  // namespace iroha::consensus::proxy

#endif
