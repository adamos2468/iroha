#include<bits/stdc++.h>
#include"consensus.hpp"
using namespace std;

int main(int argc, char** argv){
    string consensus_endpoint=argv[1];
    string network_endpoint=argv[2];
    string proxy_endpoint=argv[3];
    string peerInfo=argv[4];
    auto test=std::shared_ptr<iroha::consensus::consensusProxy>(new iroha::consensus::consensusProxy(consensus_endpoint, network_endpoint, proxy_endpoint, peerInfo));

    test->start();
}