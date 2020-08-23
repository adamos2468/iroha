/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus.hpp"

#include <cassert>

namespace {
  std::string getBlockId(shared_model::interface::Block const &block) {
    // TODO
  }
}  // namespace

namespace iroha {
  namespace consensus {
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
    consensusProxy::consensusProxy(std::string engine_endpoint,
                                   std::string network_endpoint,
                                   std::string proxy_endpoint,
                                   std::string peerInfo) {
      testInit();
      this->engine_endpoint = engine_endpoint;
      this->network_endpoint = network_endpoint;
      this->proxy_endpoint = proxy_endpoint;
      this->localPeer.set_peer_id(hexToBin(peerInfo));
      connected_peers.insert(localPeer.peer_id());
      this->myHexId = peerInfo;
    }
    void consensusProxy::testInit() {
      message::ConsensusBlock genesis;
      genesis.set_block_id(std::to_string(blockchain.size() + 1));
      genesis.set_block_num(blockchain.size());
      genesis.set_previous_id(std::to_string(blockchain.size()));
      genesis.set_summary("deadbeef");
      genesis.set_payload("payload");
      genesis.set_signer_id(localPeer.peer_id());

      this->blockchain.push_back(genesis);
      this->canditateBlock = genesis;
      this->localPeer.set_peer_id("");
      this->network_endpoint = "";
      this->engine_endpoint = "";
      this->consensus_settings = {
          {"sawtooth.consensus.pbft.members", "[\"A1\",\"A2\",\"A3\",\"A4\"]"},
          {"sawtooth.consensus.pbft.idle_timeout", "10000"}};
      // peers.push_back(localPeer);
      for (int i = 1; i <= 4; i++) {
        message::ConsensusPeerInfo temp;
        std::string peerid;
        peerid.push_back(160 + i);
        temp.set_peer_id(peerid);
        peers.push_back(temp);
      }
    }
    message::ConsensusRegisterResponse
    consensusProxy::handleConsensusRegisterReq(
        message::ConsensusRegisterRequest request) {
      this->EngineInfo.name = request.name();
      this->EngineInfo.version = request.version();
      for (auto x : request.additional_protocols())
        this->EngineInfo.additional_protocols.push_back(x);
      message::ConsensusRegisterResponse ans;
      ans.set_status(message::ConsensusRegisterResponse_Status::
                         ConsensusRegisterResponse_Status_OK);
      if (connected_peers.size() < 4) {
        ans.set_status(
            message::ConsensusRegisterResponse_Status::
                ConsensusRegisterResponse_Status_NOT_READY);  // When Peers<4
                                                              // (Need to Add
                                                              // Dynamic
                                                              // Peering)
      }
      ans.set_allocated_local_peer_info(&localPeer);
      chainhead = blockchain.back();

      for (auto x : peers) {
        message::ConsensusPeerInfo *temp = ans.add_peers();
        temp->set_peer_id(x.peer_id());
      }

      ans.set_allocated_chain_head(&chainhead);
      return ans;
    }
    // TODO READ THE SETTING TABLE
    std::string consensusProxy::getSetting(std::string setting) {
      if (consensus_settings.count(setting))
        return consensus_settings[setting];
      return "";
    }
    message::ConsensusSettingsGetResponse
    consensusProxy::handleConsensusSettingsGetReq(
        message::ConsensusSettingsGetRequest request) {
      message::ConsensusSettingsGetResponse ans;
      ans.set_status(message::ConsensusSettingsGetResponse_Status::
                         ConsensusSettingsGetResponse_Status_OK);
      for (int i = 0; i < request.keys_size(); i++) {
        std::string key = request.keys(i);
        std::string value = getSetting(key);
        if (value != "") {
          message::ConsensusSettingsEntry *temp = ans.add_entries();
          temp->set_key(key);
          temp->set_value(value);
        }
      }
      return ans;
    }
    message::ConsensusPeerMessage consensusProxy::getPeerMessage(
        std::string message_type, std::string content) {
      message::ConsensusPeerMessageHeader peermsg_header;
      peermsg_header.set_message_type(message_type);
      peermsg_header.set_name(this->EngineInfo.name);
      peermsg_header.set_version(this->EngineInfo.version);
      peermsg_header.set_content_sha512("Enter Hash here");
      peermsg_header.set_signer_id(localPeer.peer_id());
      std::string head_buf;
      peermsg_header.SerializeToString(&head_buf);

      message::ConsensusPeerMessage peermsg;
      peermsg.set_header_signature("Enter Signature Here");
      peermsg.set_header(head_buf);
      peermsg.set_content(content);
      return peermsg;
    }
    void consensusProxy::broadcast(message::ConsensusPeerMessage msg) {
      std::string msg_buf;
      msg.SerializeToString(&msg_buf);
      //! MAY NEED MUTEX!
      channel.push_msg("network", "", msg_buf);
      // network_socket->send(out);
    }
    message::ConsensusBroadcastResponse
    consensusProxy::handleConsensusBroadcastReq(
        message::ConsensusBroadcastRequest request) {
      std::cout << "\t" << request.message_type() << std::endl;
      message::ConsensusPeerMessage peermsg =
          getPeerMessage(request.message_type(), request.content());
      broadcast(peermsg);
      message::ConsensusBroadcastResponse ans;
      ans.set_status(message::ConsensusBroadcastResponse_Status::
                         ConsensusBroadcastResponse_Status_OK);
      return ans;
    }
    // TODO Create Block from iroha
    message::ConsensusBlock consensusProxy::initializeBlock() {
      message::ConsensusBlock temp;
      temp.set_block_id(std::to_string(blockchain.size() + 1));
      temp.set_payload("deadbeef");
      temp.set_signer_id(localPeer.peer_id());
      temp.set_summary("deadbeef" + std::to_string(blockchain.size()));
      temp.set_previous_id(blockchain.back().block_id());
      temp.set_block_num(blockchain.size());
      return temp;
    }

    std::optional<shared_ptr<shared_model::interface::Block>>
    consensusProxy::getCandidateBlock() const {
      return last_block_creator_event_ |
          [](auto const &last_block_creator_event) {
            return last_block_creator_event.round_data |
                [](auto const &round_data) { return round_data.block; };
          };
    }

    message::ConsensusInitializeBlockResponse
    consensusProxy::handleBlockInitReq(
        message::ConsensusInitializeBlockRequest request) {
      canditateBlock = initializeBlock();  // Create the new block
      newblock = false;
      message::ConsensusInitializeBlockResponse ans;
      ans.set_status(message::ConsensusInitializeBlockResponse_Status::
                         ConsensusInitializeBlockResponse_Status_OK);
      return ans;
    }
    message::ConsensusSummarizeBlockResponse consensusProxy::handleBlockSumReq(
        message::ConsensusSummarizeBlockRequest request) {
      message::ConsensusSummarizeBlockResponse ans;
      if (getCandidateBlock()) {
        ans.set_summary(canditateBlock.summary());
        ans.set_status(message::ConsensusSummarizeBlockResponse_Status::
                           ConsensusSummarizeBlockResponse_Status_OK);
      } else {
        ans.set_status(message::ConsensusSummarizeBlockResponse_Status::
                           ConsensusSummarizeBlockResponse_Status_NOT_READY);
      }
      ans.set_summary(canditateBlock.summary());
      ans.set_status(message::ConsensusSummarizeBlockResponse_Status::
                         ConsensusSummarizeBlockResponse_Status_OK);
      return ans;
    }
    void consensusProxy::NotifyBlockNew() {
      message::ConsensusNotifyBlockNew blocknew;
      blocknew.set_allocated_block(&canditateBlock);
      message::Message msg;
      msg.set_message_type(message::Message_MessageType::
                               Message_MessageType_CONSENSUS_NOTIFY_BLOCK_NEW);
      std::string buf;
      blocknew.SerializeToString(&buf);
      msg.set_content(buf);
      msg.set_correlation_id("randomcorelation");
      channel.push_msg("engine", engine_id, msg.SerializeAsString());
      // sendMsg(engine_socket, engine_id, msg);
      blocknew.release_block();
    }
    void consensusProxy::NotifyBlockValid(std::string blockid) {
      message::ConsensusNotifyBlockValid blockvalid;
      blockvalid.set_block_id(blockid);
      message::Message msg;
      msg.set_message_type(
          message::Message_MessageType::
              Message_MessageType_CONSENSUS_NOTIFY_BLOCK_VALID);
      std::string buf;
      blockvalid.SerializeToString(&buf);
      msg.set_content(buf);
      msg.set_correlation_id("randomcorelation");
      channel.push_msg("engine", engine_id, msg.SerializeAsString());
      // sendMsg(engine_socket, engine_id, msg);
    }
    void consensusProxy::NotifyBlockInvalid(std::string blockid) {
      message::ConsensusNotifyBlockInvalid blockinvalid;
      blockinvalid.set_block_id(blockid);
      message::Message msg;
      msg.set_message_type(
          message::Message_MessageType::
              Message_MessageType_CONSENSUS_NOTIFY_BLOCK_INVALID);
      std::string buf;
      blockinvalid.SerializeToString(&buf);
      msg.set_content(buf);
      msg.set_correlation_id("randomcorelation");
      channel.push_msg("engine", engine_id, msg.SerializeAsString());
      // sendMsg(engine_socket, engine_id, msg);
    }
    void consensusProxy::BroadCastCanditateBlock() {
      std::string buf;
      canditateBlock.SerializeToString(&buf);
      message::ConsensusPeerMessage peermsg = getPeerMessage("NewBlock", buf);
      broadcast(peermsg);
    }
    message::ConsensusFinalizeBlockResponse consensusProxy::handleBlockFinalReq(
        message::ConsensusFinalizeBlockRequest request) {
      // Need to do something with the consensus data
      // canditateBlock.set_summary(canditateBlock.summary()+"_c");
      // canditateBlock.set_block_id(canditateBlock.block_id()+"_c");
      message::ConsensusFinalizeBlockResponse ans;
      if (getCandidateBlock()) {
        BroadCastCanditateBlock();
        if (newblock == false) {
          newblock = true;
          NotifyBlockNew();
        }
        ans.set_block_id(getBlockId(*getCandidateBlock().value()));
        ans.set_status(message::ConsensusFinalizeBlockResponse_Status::
                           ConsensusFinalizeBlockResponse_Status_OK);
      } else {
        ans.set_status(message::ConsensusFinalizeBlockResponse_Status::
                           ConsensusFinalizeBlockResponse_Status_NOT_READY);
      }
      return ans;
    }
    message::ConsensusCheckBlocksResponse consensusProxy::handleBlockCheckReq(
        message::ConsensusCheckBlocksRequest request) {
      for (int i = 0; i < request.block_ids_size(); i++) {
        NotifyBlockValid(request.block_ids(i));
      }
      message::ConsensusCheckBlocksResponse response;
      response.set_status(message::ConsensusCheckBlocksResponse_Status::
                              ConsensusCheckBlocksResponse_Status_OK);
      return response;
    }
    void consensusProxy::NotifyBlockCommit(std::string blockid) {
      message::ConsensusNotifyBlockCommit notfyCommit;
      notfyCommit.set_block_id(blockid);
      std::string buf = notfyCommit.SerializeAsString();

      message::Message msg;
      msg.set_message_type(
          message::Message_MessageType::
              Message_MessageType_CONSENSUS_NOTIFY_BLOCK_COMMIT);
      msg.set_correlation_id("Hakuna Matata");
      msg.set_content(buf);
      std::string msgbuf = msg.SerializeAsString();

      channel.push_msg("engine", engine_id, msgbuf);
    }
    message::ConsensusCommitBlockResponse consensusProxy::handleBlockCommitReq(
        message::ConsensusCommitBlockRequest request) {
      auto canditate_block = getCandidateBlock();
      assert(canditate_block);  // this ensures last_block_creator_event_
      synchronizer_->processOutcome(
          PairValid{last_block_creator_event_->round,
                    last_block_creator_event_->ledger_state,
                    canditate_block});
      NotifyBlockCommit(getBlockId(*canditate_block));
      message::ConsensusCommitBlockResponse response;
      response.set_status(message::ConsensusCommitBlockResponse_Status::
                              ConsensusCommitBlockResponse_Status_OK);
      return response;
    }
    message::ConsensusFailBlockResponse consensusProxy::handleFailBlockReq(
        message::ConsensusFailBlockRequest request) {
      // Delete the block in request
      message::ConsensusFailBlockResponse response;
      response.set_status(message::ConsensusFailBlockResponse_Status::
                              ConsensusFailBlockResponse_Status_OK);
      return response;
    }

    message::Message consensusProxy::handleEngineMessage(
        message::Message request) {
      std::cerr << myHexId << " Received: "
                << Message_MessageType_Name(request.message_type())
                << std::endl;
      message::Message ans;
      std::string buf;
      ans.set_correlation_id(request.correlation_id());
      if (request.message_type()
          == message::Message::CONSENSUS_REGISTER_REQUEST) {
        message::ConsensusRegisterRequest consreq;
        consreq.ParseFromString(request.content());
        message::ConsensusRegisterResponse consrep =
            handleConsensusRegisterReq(consreq);
        consrep.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(message::Message::CONSENSUS_REGISTER_RESPONSE);
        consrep.release_chain_head();
        consrep.release_local_peer_info();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      } else if (request.message_type()
                 == message::Message::CONSENSUS_SETTINGS_GET_REQUEST) {
        message::ConsensusSettingsGetRequest setreg;
        setreg.ParseFromString(request.content());
        message::ConsensusSettingsGetResponse setres =
            handleConsensusSettingsGetReq(setreg);
        setres.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(message::Message::CONSENSUS_SETTINGS_GET_RESPONSE);
      } else if (request.message_type()
                 == message::Message::CONSENSUS_BROADCAST_REQUEST) {
        message::ConsensusBroadcastRequest broreg;
        broreg.ParseFromString(request.content());
        message::ConsensusBroadcastResponse brores =
            handleConsensusBroadcastReq(broreg);
        brores.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(message::Message::CONSENSUS_BROADCAST_RESPONSE);
      } else if (request.message_type()
                 == message::Message::CONSENSUS_INITIALIZE_BLOCK_REQUEST) {
        message::ConsensusInitializeBlockRequest initreq;
        initreq.ParseFromString(request.content());
        message::ConsensusInitializeBlockResponse initres =
            handleBlockInitReq(initreq);
        initres.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(
            message::Message::CONSENSUS_INITIALIZE_BLOCK_RESPONSE);
      } else if (request.message_type()
                 == message::Message::CONSENSUS_SUMMARIZE_BLOCK_REQUEST) {
        message::ConsensusSummarizeBlockRequest sumreq;
        sumreq.ParseFromString(request.content());
        message::ConsensusSummarizeBlockResponse sumres =
            handleBlockSumReq(sumreq);
        sumres.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(
            message::Message::CONSENSUS_SUMMARIZE_BLOCK_RESPONSE);
      } else if (request.message_type()
                 == message::Message::CONSENSUS_FINALIZE_BLOCK_REQUEST) {
        message::ConsensusFinalizeBlockRequest finalreq;
        finalreq.ParseFromString(request.content());
        message::ConsensusFinalizeBlockResponse finalres =
            handleBlockFinalReq(finalreq);
        finalres.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(
            message::Message::CONSENSUS_FINALIZE_BLOCK_RESPONSE);
      } else if (request.message_type()
                 == message::Message::CONSENSUS_CHECK_BLOCKS_REQUEST) {
        message::ConsensusCheckBlocksRequest checkreq;
        checkreq.ParseFromString(request.content());
        message::ConsensusCheckBlocksResponse checkres =
            handleBlockCheckReq(checkreq);
        checkres.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(message::Message::CONSENSUS_CHECK_BLOCKS_RESPONSE);
      } else if (request.message_type()
                 == message::Message::CONSENSUS_COMMIT_BLOCK_REQUEST) {
        message::ConsensusCommitBlockRequest commitreq;
        commitreq.ParseFromString(request.content());
        message::ConsensusCommitBlockResponse commitres =
            handleBlockCommitReq(commitreq);
        commitres.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(message::Message::CONSENSUS_COMMIT_BLOCK_RESPONSE);
      } else if (request.message_type()
                 == message::Message::CONSENSUS_FAIL_BLOCK_REQUEST) {
        message::ConsensusFailBlockRequest failreq;
        failreq.ParseFromString(request.content());
        message::ConsensusFailBlockResponse failres =
            handleFailBlockReq(failreq);
        failres.SerializeToString(&buf);
        ans.set_content(buf);
        ans.set_message_type(message::Message::CONSENSUS_FAIL_BLOCK_RESPONSE);
      }
      return ans;
    }
    /*bool consensusProxy::registerEngine(){
        std::pair<std::string, message::Message> mlprt_msg=
    rcvMsg(engine_socket); std::string con_id=mlprt_msg.first; engine_id=con_id;
        message::Message msg=mlprt_msg.second;
        if(msg.message_type()==message::Message::CONSENSUS_REGISTER_REQUEST){
            message::Message ans=handleEngineMessage(msg);
            channel.push_msg("engine", engine_id, ans.SerializeAsString());
            //sendMsg(engine_socket, con_id,ans);
            return true;
        }
        std::cerr<<"Got: "<<Message_MessageType_Name(msg.message_type())<<"
    Expected:
    "<<Message_MessageType_Name(message::Message::CONSENSUS_REGISTER_REQUEST)<<std::endl;
        return false;
    }
    void consensusProxy::handleEngine(){
        while(true){
            std::pair<std::string, message::Message> mlprt_msg=
    rcvMsg(engine_socket); std::string con_id=mlprt_msg.first; message::Message
    msg=mlprt_msg.second; message::Message ans=handleEngineMessage(msg);
            if(msg.message_type()!=message::Message_MessageType::Message_MessageType_CONSENSUS_NOTIFY_ACK){
                channel.push_msg("engine", engine_id, ans.SerializeAsString());
                //sendMsg(engine_socket, con_id,ans);
            }
        }
    }
    void consensusProxy::startEngine(){
        while(true)
            if(registerEngine()){
                handleEngine();
            }
    }*/
    void consensusProxy::NotifyPeerMsg(std::string id,
                                       message::ConsensusPeerMessage msg) {
      message::ConsensusNotifyPeerMessage notfymsg;
      notfymsg.set_allocated_message(&msg);
      notfymsg.set_sender_id(id);
      std::string buf;
      notfymsg.SerializeToString(&buf);

      message::Message toTheEngine;
      toTheEngine.set_message_type(
          message::Message_MessageType::
              Message_MessageType_CONSENSUS_NOTIFY_PEER_MESSAGE);
      toTheEngine.set_content(buf);
      toTheEngine.set_correlation_id("Hakuna Matata");
      std::string msgstr;
      toTheEngine.SerializeToString(&msgstr);
      channel.push_msg("engine", engine_id, msgstr);
      // sendMsg(engine_socket, engine_id, toTheEngine);
      notfymsg.release_message();
    }
    void consensusProxy::showPresence() {
      message::ConsensusPeerMessage peermsg =
          getPeerMessage("Bonjour!", localPeer.peer_id());
      std::string buf;
      peermsg.SerializeToString(&buf);
      channel.push_msg("network", "", buf);
      // network_socket->send(output);
    }
    void consensusProxy::handlePeerMsg(std::string id,
                                       message::ConsensusPeerMessage msg) {
      message::ConsensusPeerMessageHeader header;
      // Check for header signature
      header.ParseFromString(msg.header());
      std::cerr << myHexId << " Received " << header.message_type() << " from "
                << binToHex(id) << "\n";
      if (header.message_type() == "Bonjour!") {
        if (!connected_peers.count(msg.content())) {
          connected_peers.insert(msg.content());
          showPresence();
        }
      } else if (header.message_type() == "NewBlock") {
        message::ConsensusBlock neoblock;
        neoblock.ParseFromString(msg.content());
        if (canditateBlock.block_id() != neoblock.block_id()) {
          canditateBlock.ParseFromString(msg.content());
          NotifyBlockNew();
        }
      } else {
        NotifyPeerMsg(id, msg);
      }
    }
    void consensusProxy::start() {
      engine_socket = std::shared_ptr<zmq::socket_t>(
          new zmq::socket_t(ctx, zmq::socket_type::router));
      proxy_socket = std::shared_ptr<zmq::socket_t>(
          new zmq::socket_t(ctx, zmq::socket_type::sub));
      network_socket = std::shared_ptr<zmq::socket_t>(
          new zmq::socket_t(ctx, zmq::socket_type::dealer));

      engine_socket->bind(engine_endpoint);

      network_socket->setsockopt(ZMQ_IDENTITY,
                                 localPeer.peer_id().c_str(),
                                 localPeer.peer_id().size());
      network_socket->connect(network_endpoint);

      proxy_socket->connect(proxy_endpoint);
      proxy_socket->setsockopt(ZMQ_SUBSCRIBE, "", 0);

      channel.add_socket(proxy_socket, "proxy");
      channel.add_socket(engine_socket, "engine");
      channel.add_socket(network_socket, "network");
      std::thread channel_th(&zmq_channel::start, &channel);
      showPresence();
      while (true) {
        std::string socket, id, msg;
        if (channel.pop_msg(socket, id, msg)) {
          if (id == localPeer.peer_id())
            continue;
          if (socket == "proxy") {
            message::ConsensusPeerMessage peermsg;
            peermsg.ParseFromString(msg);
            handlePeerMsg(id, peermsg);
          } else if (socket == "engine") {
            engine_id = id;
            message::Message response, request;
            request.ParseFromString(msg);
            if (request.message_type()
                == message::Message_MessageType::
                       Message_MessageType_CONSENSUS_NOTIFY_ACK)
              continue;
            response = handleEngineMessage(request);
            channel.push_msg("engine", id, response.SerializeAsString());
          }
        }
      }
      channel_th.join();
    }
  }  // namespace consensus
}  // namespace iroha
