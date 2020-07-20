#include"consensus.hpp"
namespace iroha{
    namespace consensus{
        consensusProxy::consensusProxy(std::string engine_endpoint, std::string network_endpoint, std::string proxy_endpoint, std::string peerInfo){
            testInit();
            this->engine_endpoint=engine_endpoint;
            this->network_endpoint=network_endpoint;
            this->proxy_endpoint=proxy_endpoint;
            this->localPeer.set_peer_id(peerInfo);
        }
        void consensusProxy::testInit(){
            message::ConsensusBlock genesis;
            genesis.set_block_id(std::to_string(blockchain.size()+1));
            genesis.set_block_num(blockchain.size());
            genesis.set_previous_id(std::to_string(blockchain.size()));
            genesis.set_summary("deadbeef");
            genesis.set_payload("payload");
            genesis.set_signer_id("A1");

            this->blockchain.push_back(genesis);
            this->canditateBlock=genesis;
            this->localPeer.set_peer_id("");
            this->network_endpoint="";
            this->engine_endpoint="";
            this->consensus_settings={
                {"sawtooth.consensus.pbft.members", "[\"A1\",\"A2\",\"A3\",\"A4\"]"},
                {"sawtooth.consensus.pbft.idle_timeout", "5000"}
            };

            for(int i=1; i<=4; i++){
                message::ConsensusPeerInfo temp;
                temp.set_peer_id("A"+std::to_string(i));
                peers.push_back(temp);    
            }
        }
        std::pair<std::string, message::Message> consensusProxy::rcvMsg(std::shared_ptr<zmqpp::socket> socket){
            std::string id, msg;
            message::Message dec;
            zmqpp::message zmq_message;
            socket->receive(zmq_message);
            zmq_message>>id>>msg;
            dec.ParseFromString(msg);
            return {id, dec};
        }
        bool consensusProxy::sendMsg(std::shared_ptr<zmqpp::socket> socket ,std::string id ,message::Message msg){
            std::string buf;
            msg.SerializeToString(&buf);
            zmqpp::message reply;
            reply<<id;
            reply<<buf;
            return socket->send(reply);
        }
        message::ConsensusRegisterResponse consensusProxy::handleConsensusRegisterReq(message::ConsensusRegisterRequest request){
            this->EngineInfo.name=request.name();
            this->EngineInfo.version=request.version();
            for(auto x:request.additional_protocols())
                this->EngineInfo.additional_protocols.push_back(x);
            message::ConsensusRegisterResponse ans;
            ans.set_status(message::ConsensusRegisterResponse_Status::ConsensusRegisterResponse_Status_OK);
            ans.set_allocated_local_peer_info(&localPeer);
            chainhead=blockchain.back();
            
            for(auto x:peers){
                message::ConsensusPeerInfo* temp=ans.add_peers();
                temp->set_peer_id(x.peer_id());
            }

            ans.set_allocated_chain_head(&chainhead);
            return ans;
        }
        //TODO READ THE SETTING TABLE
        std::string consensusProxy::getSetting(std::string setting){
            if(consensus_settings.count(setting))
                return consensus_settings[setting];
            return "";
        }
        message::ConsensusSettingsGetResponse consensusProxy::handleConsensusSettingsGetReq(message::ConsensusSettingsGetRequest request){
            message::ConsensusSettingsGetResponse ans;
            ans.set_status(message::ConsensusSettingsGetResponse_Status::ConsensusSettingsGetResponse_Status_OK);
            for(int i=0; i<request.keys_size(); i++){
                std::string key=request.keys(i);
                std::string value=getSetting(key);
                if(value!=""){
                    message::ConsensusSettingsEntry *temp=ans.add_entries();
                    temp->set_key(key);
                    temp->set_value(value);
                }
            }
            return ans;
        }
        message::ConsensusBroadcastResponse consensusProxy::handleConsensusBroadcastReq(message::ConsensusBroadcastRequest request){
            message::ConsensusPeerMessageHeader peermsgheader;
            peermsgheader.set_message_type(request.message_type());
            peermsgheader.set_name(this->EngineInfo.name);
            peermsgheader.set_version(this->EngineInfo.version);
            peermsgheader.set_content_sha512("jsbshjs");


            message::ConsensusPeerMessage peermsg;


            message::pbft::PbftNewView test;
            test.ParseFromString(request.content());
            message::pbft::PbftMessageInfo hmm;
            hmm=test.info();
            std::cout<<hmm.msg_type()<<" "<<hmm.signer_id()<<" "<<hmm.view()<<"\n";
            message::ConsensusBroadcastResponse ans;
            ans.set_status(message::ConsensusBroadcastResponse_Status::ConsensusBroadcastResponse_Status_OK);
            return ans;
        }
        message::Message consensusProxy::handleEngineMessage(message::Message request){
            message::Message ans;
            ans.set_correlation_id(request.correlation_id());
            if(request.message_type()==message::Message::CONSENSUS_REGISTER_REQUEST){
                message::ConsensusRegisterRequest consreq;
                consreq.ParseFromString(request.content());
                message::ConsensusRegisterResponse consrep=handleConsensusRegisterReq(consreq);
                std::string buf="";
                consrep.SerializeToString(&buf);
                ans.set_content(buf);
                ans.set_message_type(message::Message::CONSENSUS_REGISTER_RESPONSE);
                consrep.release_chain_head();
                consrep.release_local_peer_info();
            }else if(request.message_type()==message::Message::CONSENSUS_SETTINGS_GET_REQUEST){
                message::ConsensusSettingsGetRequest setreg;
                setreg.ParseFromString(request.content());
                message::ConsensusSettingsGetResponse setres=handleConsensusSettingsGetReq(setreg);
                std::string buf="";
                setres.SerializeToString(&buf);
                ans.set_content(buf);
                ans.set_message_type(message::Message::CONSENSUS_SETTINGS_GET_RESPONSE);
            }else if(request.message_type()==message::Message::CONSENSUS_BROADCAST_REQUEST){
                message::ConsensusBroadcastRequest broreg;
                broreg.ParseFromString(request.content());
                message::ConsensusBroadcastResponse brores=handleConsensusBroadcastReq(broreg);
                std::string buf="";
                brores.SerializeToString(&buf);
                ans.set_content(buf);
                ans.set_message_type(message::Message::CONSENSUS_BROADCAST_RESPONSE);
            }
            return ans;
        }
        bool consensusProxy::registerEngine(){
            std::pair<std::string, message::Message> mlprt_msg= rcvMsg(engine_socket);
            std::string con_id=mlprt_msg.first;
            message::Message msg=mlprt_msg.second;
            if(msg.message_type()==message::Message::CONSENSUS_REGISTER_REQUEST){
                std::cerr<<"Received: "<<Message_MessageType_Name(msg.message_type())<<std::endl;
                message::Message ans=handleEngineMessage(msg);
                sendMsg(engine_socket, con_id,ans);
                return true;
            }
            std::cerr<<"Got: "<<Message_MessageType_Name(msg.message_type())<<" Expected: "<<Message_MessageType_Name(message::Message::CONSENSUS_REGISTER_REQUEST)<<std::endl;
            return false;
        }
        void consensusProxy::handleEngine(){
            while(true){
                std::pair<std::string, message::Message> mlprt_msg= rcvMsg(engine_socket);
                std::string con_id=mlprt_msg.first;
                message::Message msg=mlprt_msg.second;
                std::cerr<<"Received: "<<Message_MessageType_Name(msg.message_type())<<std::endl;
                message::Message ans=handleEngineMessage(msg);
                sendMsg(engine_socket, con_id,ans);
            }
        }
        void consensusProxy::startEngine(){
            while(true)
                if(registerEngine()){
                    handleEngine();
                }
        }
        void consensusProxy::start(){
            engine_socket=std::shared_ptr<zmqpp::socket> (new zmqpp::socket(ctx, zmqpp::socket_type::router));
            proxy_socket=std::shared_ptr<zmqpp::socket> (new zmqpp::socket(ctx, zmqpp::socket_type::sub));
            network_socket=std::shared_ptr<zmqpp::socket> (new zmqpp::socket(ctx, zmqpp::socket_type::dealer));
            network_socket->set(zmqpp::socket_option::identity, localPeer.peer_id());

            //printf("%s\n", engine_endpoint.c_str());
            engine_socket->bind(engine_endpoint);
            std::thread engine_th(&consensusProxy::startEngine, this);
            engine_th.join();
        }
    }
}