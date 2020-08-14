/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/proxy/consensus_proxy.hpp"

#include <future>
#include <optional>

#include "common/bind.hpp"
#include "ordering/impl/on_demand_common.hpp"
#include "simulator/impl/simulator.hpp"

using namespace iroha::consensus::proxy;

using iroha::operator|;

ConsensusInitializeBlockResponse Proxy::handleInitializeBlock(
    ConsensusInitializeBlockRequest const &request) {
  ConsensusInitializeBlockResponse response;
  auto proposal = ordering_network_client_->requestProposal(ordering_round_);
  if (not proposal) {
    ordering_round_ = iroha::ordering::nextRejectRound(ordering_round_);
    response.status = NOT_READY;
    return response;
  }

  candidate_block_ = PromisedBlock{std::async(
      std::launch::async,
      [proposal = std::move(proposal), simulator = simulator_] {
        auto verified_proposal_and_errors =
            simulator->processProposal(*proposal);
        return verified_proposal_and_errors.verified_proposal_result |
            [&verified_proposal_and_errors](auto const &verified_proposal) {
              return simulator->processVerifiedProposal(
                  verified_proposal,
                  verified_proposal_and_errors.ledger_state->top_block_info);
            }
      })};

  response.status = OK;
  return response;
}

ConsensusSummarizeBlockResponse Proxy::handleSummarizeBlock(
    ConsensusSummarizeBlockRequest const &request) {
  ConsensusSummarizeBlockResponse response;
  if (candidate_block_ and candidate_block_->get()) {
    response.status = OK;
    response.summary = candidate_block_->get().value()->hash();
  } else {
    ordering_round_ = iroha::ordering::nextRejectRound(ordering_round_);
    response.status = NOT_READY;
  }
  return response;
}

ConsensusFinalizeBlockResponse Proxy::handleFinalizeBlock(
    ConsensusFinalizeBlockRequest const &request) {
  request.data ConsensusFinalizeBlockResponse response;
  if (candidate_block_ and candidate_block_->get()) {
    response.status = OK;
    candidate_block_->get().value()->addConsensusSeal(request.data);
    response.block_id = candidate_block_->get().value()->height();
  } else {
    ordering_round_ = iroha::ordering::nextRejectRound(ordering_round_);
    response.status = NOT_READY;
  }
  return response;
}

ConsensusCancelBlockResponse Proxy::handleConsensusCancelBlock(
    ConsensusCancelBlockRequest const &request) {
  candidate_block_ = std::nullopt;
  ordering_round_ = iroha::ordering::nextRejectRound(ordering_round_);
  ConsensusCancelBlockResponse response;
  response.status = OK;
  return response;
}

ConsensusCheckBlocksResponse Proxy::handleCheckBlocks(
    ConsensusCheckBlocksRequest const &request) {
  request.block_ids;
  // TODO
  ConsensusCheckBlocksResponse response;
  response.status
}

ConsensusCommitBlockResponse Proxy::handleCommitBlock(
    ConsensusCommitBlockRequest const &request) {
  ordering_round_ = iroha::ordering::nextCommitRound(ordering_round_);
  request.block_id;
  // TODO
  ConsensusCommitBlockResponse response;
  response.status
}

Proxy::PromisedBlock::PromisedBlock(
    std::future<BlockCreationResult> future_block)
    : promised_block_(std::move(future_block)) {}

BlockCreationResult &Proxy::PromisedBlock::get() {
  if (not block_) {
    block_ = promised_block_.get();
  }
  return block_.value();
}
